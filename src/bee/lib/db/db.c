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


// low level functions
int db_open   (char * file)
{  int rc;

   rc = open(file, O_RDWR, 0600);
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
   if ((rem = fs.st_size%len) != 0)
   {  syslog(LOG_ERR, "db_reccount(): Warning: remandier=%d", rem);
   }
   recs=fs.st_size/len;

   return recs;
}

int db_get    (int fd, int rec, void * buf, int len)
{  int    recs;
   int    bytes;

/* get no of records */
    recs = db_reccount(fd, len);
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
    recs = db_reccount(fd, len);
    if (recs == (-1)) return IO_ERROR;
/* check size */
    if (recs <= rec) return NOT_FOUND;
/* seek to record */
    if (lseek(fd, rec*len, SEEK_SET)<0)
    {  syslog(LOG_ERR, "db_put(lseek): %m");
       return IO_ERROR;
    }
/* write record */    
    bytes = write(fd, data, len);
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
    if ((recs = db_reccount(fd, len)) == (-1)) return IO_ERROR;
/* seek to next record */
    if (lseek(fd, recs*len, SEEK_SET) < 0)
    {  syslog(LOG_ERR, "db_add(lseek): %m");
       return IO_ERROR;
    }
/* write record */    
    bytes = write(fd, data, len);
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

int db_sync   (int fd)
{   return fsync(fd);           }

int count_crc (void * data, int len)
{  uLong crc;

   crc = crc32(0L, Z_NULL, 0);
   crc = crc32(crc, data, len);

   return (int)crc;
}

