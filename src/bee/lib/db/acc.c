#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <zlib.h>

#include <bee.h>
#include <db.h>

// Log groupping step 
int   LogStep = 3600; // 1 hour

// Account base functions
int acc_baseopen (accbase_t * base, char * file)
{  int rc;
   
   rc = db_open(file);
   if (rc < 0) return rc;

   base->fd     = rc;
   base->fasync = 0;

   return SUCCESS; 
}

int acc_baseclose(accbase_t * base)
{   return db_close(base->fd);  }

int acc_reccount (accbase_t * base)
{   return db_reccount(base->fd, sizeof(acc_t));  }

int acc_baselock (accbase_t * base)
{  
   if (base->fasync == 0) return db_exlock(base->fd);
   return 0;
}

int acc_baseunlock (accbase_t * base)
{
  if (base->fasync == 0) return db_unlock(base->fd);
  return 0;
}

int acc_get      (accbase_t * base, int rec, acc_t * acc)
{  int rc;

   if (base->fasync == 0) rc = db_shlock(base->fd);
   else rc = 0;
 
   if (rc >= 0)
   {  rc = acci_get(base, rec, acc);
      if (base->fasync == 0) db_unlock(base->fd);
   }
   else return IO_ERROR;  

   return rc;
}

int acc_put      (accbase_t * base, int rec, acc_t * acc)
{  int rc;

   if (base->fasync == 0) rc = db_exlock(base->fd);
   else rc = 0;

   if (rc >= 0)
   {  rc = acci_put(base, rec, acc);
      if (base->fasync == 0) db_unlock(base->fd);
   }
   else return IO_ERROR;

   return rc;
}

int acc_add      (accbase_t * base, acc_t * acc)
{  int    rc;
   int    no = (-1);

   if (base->fasync == 0) rc = db_exlock(base->fd);
   else rc = 0;

   if (rc < 0) return rc;

   while (1)  // fictive cycle
   {  rc = acc_reccount(base);
      if (rc < 0) break;

      no         = rc;
      acc->accno = no;
      acc->crc   = count_crc(acc, sizeof(*acc) - sizeof(acc->crc));
  
      rc = db_add(base->fd, acc, sizeof(acc_t));
      if (rc == IO_ERROR) break;
      
      if (rc != no)
         syslog(LOG_ERR, "acc_add(db_add): unexpected recno"
         " (account is broken)");

      if (base->fasync == 0)
         rc = db_sync(base->fd);

      break; 
   }
   
   if (base->fasync == 0) db_unlock(base->fd);

   if (rc < 0) return rc;
   
   return no;       
}

/* 
 * >> get account
 * >> increase account
 * >> update account
 * >> log transaction
 * >> check account balance
 * >> return balance state
 */

int acc_trans    (accbase_t * base, int rec, money_t sum, 
                  is_data_t * isdata, logbase_t * logbase)
{  int      rc;
   acc_t    acc;
   logrec_t logrec;
   logrec_t oldrec;
   int      i;
   int      recs;
   
   memset(&logrec, 0, sizeof(logrec));

   rc = acc_baselock(base);
   if (rc >= 0)
   {
// log fixed fields
      logrec.time  = time(NULL);   // stub
      logrec.accno = rec;
      logrec.sum   = sum;
      if (isdata != NULL) logrec.isdata = *isdata;
      else  logrec.isdata.res_id = (-1);
// get account
      for (i=0; i<3; i++)
         if ((rc = acci_get(base, rec, &acc)) != IO_ERROR) break;
      logrec.errno = rc;
// if account is not broken, store balance
      if (rc >= 0 || rc <= ACC_FROZEN) logrec.balance = acc.balance;
// if account in valid (not frozen) count new balance
      if (rc == SUCCESS || rc == NEGATIVE || rc == ACC_OFF) acc.balance += sum;
// if account is valid - write account back
      if (rc >= 0 || rc == ACC_OFF)
      {  for (i=0; i<3; i++)
            if ((rc = acci_put(base, rec, &acc)) != IO_ERROR) break;
// if write insuccess - log error
         if (rc < 0) logrec.errno = rc;
      } 
      if ((rc = log_baselock(logbase)) == SUCCESS) 
      {  recs = log_reccount(logbase);
         rc = (-1);
         for (i=recs-1; i>=0; i--)
         {  if ((rc = logi_get(logbase, i, &oldrec)) == SUCCESS)
            {  if (logrec.time - oldrec.time < LogStep)
               {  if (logrec.accno == oldrec.accno                 &&
                      logrec.errno == oldrec.errno                 &&
                  logrec.isdata.res_id == oldrec.isdata.res_id     &&
                  logrec.isdata.user_id == oldrec.isdata.user_id   &&
                  logrec.isdata.proto_id == oldrec.isdata.proto_id &&
                  logrec.isdata.proto2 == oldrec.isdata.proto2     &&
                  logrec.isdata.host.s_addr == oldrec.isdata.host.s_addr)
                  {  oldrec.isdata.value += logrec.isdata.value;
                     oldrec.sum += logrec.sum;
                     oldrec.balance = BALANCE_NA;
                     rc = logi_put(logbase, i, &oldrec);
                     break;
                  }
                  rc = (-1);
               } 
               else 
               {  rc = (-1); 
                  break;
               }       
            }
         }
         log_baseunlock(logbase);
      } 
      if (rc < 0) rc = log_add(logbase, &logrec);
      acc_baseunlock(base);
   } else return IO_ERROR;
// return error or account state
   if (rc < 0) return rc;    
   return (acc.balance<0.01);
}   

// internal
int acci_get      (accbase_t * base, int rec, acc_t * acc)
{  int     rc;
   time_t  ctime;

   ctime = time(NULL);

/* get record */
    rc = db_get(base->fd, rec, acc, sizeof(acc_t));
    if (rc < 0) return rc;
/* check deleted mark */
    if (acc->tag & ATAG_DELETED) return ACC_DELETED;   
/* check validity */
    if (acc->tag & ATAG_BROKEN)
    {  syslog(LOG_ERR, "#%d: break flag set", rec); 
       return ACC_BROKEN;
    } 
    if (acc->crc != count_crc(acc, sizeof(acc_t) - sizeof(acc->crc)))
    {  acc->tag |= ATAG_BROKEN;
       syslog(LOG_ERR, "#%d: CRC error expect:%08x got:%08x", rec,
              count_crc(acc, sizeof(acc_t)-sizeof(acc->crc)),
              acc->crc); 
       return ACC_BROKEN;
    }
    if (acc->tag & ATAG_FROZEN) return ACC_FROZEN;
    if (acc->tag & ATAG_OFF) return ACC_OFF;
    if (acc->accno != rec)
    {  
       syslog(LOG_ERR, "#%d: internal # error expect:%d got:%d", rec,
              rec, acc->accno); 
       acc->tag |= ATAG_BROKEN;
       return ACC_BROKEN;
    }
/* success return */
    if (acc->tag & ATAG_UNLIMIT) return ACC_UNLIMIT;
    
    if (acc->start != 0 || acc->stop != 0)
    {  if (acc->start < ctime &&
           (acc->stop > ctime || acc->stop == 0)) 
          return SUCCESS;
       else 
          return NEGATIVE;
    }
    return acc->balance < 0.01 ? NEGATIVE : SUCCESS;
}

int acci_put     (accbase_t * base, int rec, acc_t * acc)
{  int rc;

    if (acc->accno != rec)
    {  syslog(LOG_ERR, "ATTEMPT to write #%d with invalid internal number %d",
               rec, acc->accno);
       return IO_ERROR;
    }
/* count CRC */
    if ((acc->tag & ATAG_BROKEN) == 0) 
       acc->crc = count_crc(acc, sizeof(acc_t)-sizeof(int));
/* write attempt */
    rc = db_put(base->fd, rec, acc, sizeof(acc_t));
    if (rc < 0) return rc;
/* syncrinize db */
    if (base->fasync == 0) 
    {  rc = db_sync(base->fd);
       if (rc < 0) return rc;
    } 
/* return success */
    return SUCCESS;       
}

int acc_async_on   (accbase_t * base)
{  int   rc;

   if (base->fasync != 0)
   {  syslog(LOG_ERR, "acc_async_on(): nested lock attempt");
      return IO_ERROR;
   }

   rc = db_exlock(base->fd);
   if (rc < 0) return rc;

   base->fasync = 1;

   return 0;
}

int acc_async_off  (accbase_t * base)
{  int   rc;

   if (base->fasync == 0)
   {  syslog(LOG_ERR, "acc_async_off(): not locked");
      return IO_ERROR;
   }
   
   db_sync(base->fd);

   rc = db_unlock(base->fd);
   if (rc < 0) return rc;

   base->fasync = 0;

   return 0;
}

