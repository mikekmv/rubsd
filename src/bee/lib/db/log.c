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

//int LogStep=3600; // 1 hour step

// Transaction log base functions
int log_baseopen (logbase_t * base, char * file)
{  int rc;
   
   rc = db_open(file);
   if (rc < 0) return rc;

   base->fd     = rc;
   base->fasync = 0;
   return SUCCESS; 
}

int log_baseclose(logbase_t * base)
{   return db_close(base->fd);  }

int log_baselock(logbase_t * base)
{  
   if (base->fasync == 0) return db_exlock(base->fd);
   return 0;   
}

int log_baseunlock(logbase_t * base)
{
   if (base->fasync == 0) return db_unlock(base->fd);
   return 0;
}


int log_reccount (logbase_t * base)
{   return db_reccount(base->fd, sizeof(logrec_t)); }

int log_get      (logbase_t * base, int rec, logrec_t * data)
{  int rc;

   if (base->fasync == 0) rc = db_shlock(base->fd);
   else rc = 0;

   if (rc >= 0)
   {  rc = logi_get(base, rec, data);
      if (base->fasync == 0) db_unlock(base->fd);
   }
   else return IO_ERROR;  
   return rc;
}

int log_put      (logbase_t * base, int rec, logrec_t * data)
{  int rc;

   syslog(LOG_ERR, "log_put() requested :)");

   if (base->fasync == 0) rc = db_exlock(base->fd);
   else rc = 0;

   if (rc >= 0)
   {  rc = logi_put(base, rec, data);
      if (base->fasync == 0) db_unlock(base->fd);
   }
   else return IO_ERROR;  
   return rc;
}

int log_add      (logbase_t * base, logrec_t * data)
{  int rc;

   if (base->fasync == 0) rc = db_exlock(base->fd);
   else rc = 0;

   if (rc >= 0)
   {  data->crc = count_crc(data, sizeof(logrec_t)-sizeof(int));

      rc = db_add(base->fd, data, sizeof(logrec_t));

      if (base->fasync == 0) db_unlock(base->fd);
   }
   return rc;       
}

// internal
int logi_get      (logbase_t * base, int rec, logrec_t * data)
{  int rc;

/* get record */
    rc = db_get(base->fd, rec, data, sizeof(logrec_t));
    if (rc < 0) return rc;
/* check validity */
    if (count_crc(data, sizeof(logrec_t) - sizeof(long)) != data->crc)
       return ACC_BROKEN;
/* success return */
    return SUCCESS;
}

int logi_put     (logbase_t * base, int rec, logrec_t * data)
{  int rc;

/* count CRC */
    data->crc = count_crc(data, sizeof(logrec_t)-sizeof(int));
/* write attempt */
    rc = db_put(base->fd, rec, data, sizeof(logrec_t));
    if (rc < 0) return rc;
/* return success */
    return SUCCESS;       
}

int log_async_on   (logbase_t * base)
{  int   rc;

   if (base->fasync != 0)
   {  syslog(LOG_ERR, "log_async_on(): nested lock attempt");
      return IO_ERROR;
   }

   rc = db_exlock(base->fd);
   if (rc < 0) return rc;

   base->fasync = 1;

   return 0;
}


int log_async_off  (logbase_t * base)
{  int   rc;

   if (base->fasync == 0)
   {  syslog(LOG_ERR, "log_async_off(): not locked");
      return IO_ERROR;
   }

   db_sync(base->fd);

   rc = db_unlock(base->fd);
   if (rc < 0) return rc;

   base->fasync = 0;

   return 0;
}

