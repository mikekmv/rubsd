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

int LogStep=3600; // 1 hour step

// low level functions
int db_open   (char * file)
{  int rc;

   rc=open(file, O_RDWR, 0600);
   if (rc < 0) syslog(LOG_ERR, "db_open(%s): %m", file);
   return rc; 
}

int db_close    (int fd)
{   return close(fd);  }

int db_reccount (int fd, int len)
{  struct stat fs;
   int         recs;
   int         rem;

   if (fstat(fd, &fs) != 0)
   {  syslog(LOG_ERR, "db_reccount(fstat): %m");
      return (-1);
   }
   if ((rem=fs.st_size%len) != 0)
   {  syslog(LOG_ERR, "db_reccount(): Warning: remandier=%d", rem);
   }
   recs=fs.st_size/len;

   return recs;
}

int db_get    (int fd, int rec, void * buf, int len)
{  int    recs;
   int    bytes;

/* get no of records */
    recs=db_reccount(fd, len);
    if (recs == (-1)) return IO_ERROR;
/* check size */
    if (recs <= rec) return NOT_FOUND;
/* seek to record */
    if (lseek(fd, rec*len, SEEK_SET)<0)
    {  syslog(LOG_ERR, "db_get(lseek): %m");
       return IO_ERROR;
    }
/* read record */    
    bytes=read(fd, buf, len);
    if (bytes < 0)
    {  syslog(LOG_ERR, "db_get(read): %m");
       return IO_ERROR;
    }
/* check for partial read */    
    if (bytes < len)
    {  syslog(LOG_ERR, "db_get(read): Partial read %d (%d) at %d rec",
       bytes, len, rec);
       return IO_ERROR;
    }
    return SUCCESS;
}

int db_put    (int fd, int rec, void * data, int len)
{   int recs;
    int bytes;

/* get no of records */
    recs=db_reccount(fd, len);
    if (recs == (-1)) return IO_ERROR;
/* check size */
    if (recs <= rec) return NOT_FOUND;
/* seek to record */
    if (lseek(fd, rec*len, SEEK_SET)<0)
    {  syslog(LOG_ERR, "db_put(lseek): %m");
       return IO_ERROR;
    }
/* write record */    
    bytes=write(fd, data, len);
    if (bytes < 0)
    {  syslog(LOG_ERR, "db_put(write): %m");
       return IO_ERROR;
    }
/* check for partial write */    
    if (bytes < len)
    {  syslog(LOG_ERR, "db_put(read): Partial write %d (%d) at %d rec",
       bytes, len, rec);
       return IO_ERROR;
    }
    return SUCCESS;
}

int db_add    (int fd, void * data, int len)
{   int recs;
    int bytes;
   
/* get no of records */
    if ((recs=db_reccount(fd, len)) == (-1)) return IO_ERROR;
/* seek to next record */
    if (lseek(fd, recs*len, SEEK_SET)<0)
    {  syslog(LOG_ERR, "db_add(lseek): %m");
       return IO_ERROR;
    }
/* write record */    
    bytes=write(fd, data, len);
    if (bytes < 0)
    {  syslog(LOG_ERR, "db_add(write): %m");
       return IO_ERROR;
    }
/* check for partial write */    
    if (bytes < len)
    {  syslog(LOG_ERR, "db_add(write): Partial write %d (%d)",
       bytes, len);
       return IO_ERROR;
    }
    return recs;
}

int db_shlock (int fd)
{   return flock(fd, LOCK_SH);  }

int db_exlock (int fd)
{   return flock(fd, LOCK_EX);  }

int db_unlock (int fd)
{   return flock(fd, LOCK_UN);  }

int count_crc (void * data, int len)
{  uLong crc=crc32(0L, Z_NULL, 0);
   crc=crc32(crc, data, len);
   return (int)crc;
}

// Account base functions
int acc_baseopen (accbase_t * base, char * file)
{  int rc;
   
   rc=db_open(file);
   if (rc < 0) return rc;
   base->fd=rc;
   return SUCCESS; 
}

int acc_baseclose(accbase_t * base)
{   return db_close(base->fd);  }

int acc_reccount (accbase_t * base)
{   return db_reccount(base->fd, sizeof(acc_t));
}

int acc_baselock (accbase_t * base)
{  return db_exlock(base->fd);
}

int acc_baseunlock (accbase_t * base)
{  return db_unlock(base->fd);
}

int acc_get      (accbase_t * base, int rec, acc_t * acc)
{  int rc;

   rc=db_shlock(base->fd);
   if (rc >= 0)
   {  rc=acci_get(base, rec, acc);
      db_unlock(base->fd);
   }
   else return IO_ERROR;  
   return rc;
}

int acc_put      (accbase_t * base, int rec, acc_t * acc)
{  int rc;
   rc=db_exlock(base->fd);
   if (rc >= 0)
   {  rc=acci_put(base, rec, acc);
      db_unlock(base->fd);
   }
   else return IO_ERROR;  
   return rc;
}

int acc_add      (accbase_t * base, acc_t * acc)
{  int rc;
   acc_t  worksp;
   int    no=(-1);

   rc=db_exlock(base->fd);
   if (rc < 0) return rc;
   rc=db_add(base->fd, acc, sizeof(acc_t));
   if (rc==IO_ERROR) return IO_ERROR;
   no=rc;
   worksp=*acc;
   worksp.accno=rc;
   rc=acci_put(base, rc, &worksp);
   db_unlock(base->fd);
   
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
   rc=acc_baselock(base);
   if (rc>=0)
   {
// log fixed fields
      logrec.time=time(NULL);   // stub
      logrec.accno=rec;
      logrec.sum=sum;
      if (isdata!=NULL) logrec.isdata=*isdata;
      else  logrec.isdata.res_id =(-1);
// get account
      for (i=0; i<3; i++)
         if ((rc=acci_get(base, rec, &acc))!=IO_ERROR) break;
      logrec.errno=rc;
// if account is not broken, store balance
      if (rc>=0 || rc<=ACC_FROZEN) logrec.balance=acc.balance;
// if account in valid (not frozen) count new balance
      if (rc == SUCCESS || rc == NEGATIVE || rc == ACC_OFF) acc.balance+=sum;
// if account is valid - write account back
      if (rc >= 0 || rc == ACC_OFF)
      {  for (i=0; i<3; i++)
            if ((rc=acci_put(base, rec, &acc))!=IO_ERROR) break;
// if write insuccess - log error
         if (rc<0) logrec.errno=rc;
      } 
      if ((rc=log_baselock(logbase)) == SUCCESS) 
      {  recs=log_reccount(logbase);
         rc=-1;
         for (i=recs-1; i>=0; i--)
         {  if ((rc=logi_get(logbase, i, &oldrec)) == SUCCESS)
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
                     rc=logi_put(logbase, i, &oldrec);
                     break;
                  }
                  rc=-1;
               } 
               else 
               {  rc=-1; 
                  break;
               }       
            }
         }
         log_baseunlock(logbase);
      } 
      if (rc<0) rc=log_add(logbase, &logrec);
      acc_baseunlock(base);
   } else return IO_ERROR;
// return error or account state
   if (rc<0) return rc;    
   return (acc.balance<0.01);
}   

// internal
int acci_get      (accbase_t * base, int rec, acc_t * acc)
{  int rc;
   time_t  ctime;

   ctime=time(NULL);

/* get record */
    rc=db_get(base->fd, rec, acc, sizeof(acc_t));
    if (rc<0) return rc;
/* check deleted mark */
    if (acc->tag & ATAG_DELETED) return ACC_DELETED;   
/* check validity */
    if (acc->tag & ATAG_BROKEN)
    {  syslog(LOG_ERR, "#%d: break flag set", rec); 
       return ACC_BROKEN;
    } 
    if (acc->crc != count_crc(acc, sizeof(acc_t)-sizeof(acc->crc)))
    {  acc->tag |= ATAG_BROKEN;
       syslog(LOG_ERR, "#%d: CRC error expect:%08x got:%08x", rec,
              count_crc(acc, sizeof(acc_t)-sizeof(acc->crc)),
              acc->crc); 
       return ACC_BROKEN;
    }
    if (acc->tag & ATAG_FROZEN) return ACC_FROZEN;
    if (acc->tag & ATAG_OFF) return ACC_OFF;
    if (acc->accno!=rec)
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
    return acc->balance<0.01 ? NEGATIVE : SUCCESS;
}

int acci_put     (accbase_t * base, int rec, acc_t * acc)
{  int rc;

/* count CRC */
    if ((acc->tag & ATAG_BROKEN) == 0) 
       acc->crc = count_crc(acc, sizeof(acc_t)-sizeof(int));
/* write attempt */
    rc=db_put(base->fd, rec, acc, sizeof(acc_t));
    if (rc<0) return rc;
/* return success */
    return SUCCESS;       
}

// Transaction log base functions
int log_baseopen (logbase_t * base, char * file)
{  int rc;
   
   rc=db_open(file);
   if (rc < 0) return rc;
   base->fd=rc;
   return SUCCESS; 
}

int log_baseclose(logbase_t * base)
{   return db_close(base->fd);  }

int log_baselock(logbase_t * base)
{  return db_exlock(base->fd);
}

int log_baseunlock(logbase_t * base)
{  return db_unlock(base->fd);
}


int log_reccount (logbase_t * base)
{   return db_reccount(base->fd, sizeof(logrec_t)); }

int log_get      (logbase_t * base, int rec, logrec_t * data)
{  int rc;

   rc=db_shlock(base->fd);
   if (rc >= 0)
   {  rc=logi_get(base, rec, data);
      db_unlock(base->fd);
   }
   else return IO_ERROR;  
   return rc;
}

int log_put      (logbase_t * base, int rec, logrec_t * data)
{  int rc;

   syslog(LOG_ERR, "!!! log_put() requested !!!");
   rc=db_exlock(base->fd);
   if (rc >= 0)
   {  rc=logi_put(base, rec, data);
      db_unlock(base->fd);
   }
   else return IO_ERROR;  
   return rc;
}

int log_add      (logbase_t * base, logrec_t * data)
{  int rc;

   rc=db_exlock(base->fd);
   if (rc >= 0)
   {  data->crc=count_crc(data, sizeof(logrec_t)-sizeof(int));
      rc=db_add(base->fd, data, sizeof(logrec_t));
      db_unlock(base->fd);
   }
   return rc;       
}

// internal
int logi_get      (logbase_t * base, int rec, logrec_t * data)
{  int rc;

/* get record */
    rc=db_get(base->fd, rec, data, sizeof(logrec_t));
    if (rc<0) return rc;
/* check validity */
    if (data->crc!=count_crc(data, sizeof(logrec_t)-sizeof(int)))
       return ACC_BROKEN;
/* success return */
    return SUCCESS;
}

int logi_put     (logbase_t * base, int rec, logrec_t * data)
{  int rc;

/* count CRC */
    data->crc=count_crc(data, sizeof(logrec_t)-sizeof(int));
/* write attempt */
    rc=db_put(base->fd, rec, data, sizeof(logrec_t));
    if (rc<0) return rc;
/* return success */
    return SUCCESS;       
}
