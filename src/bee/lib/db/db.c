#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>

#include <bee.h>
#include <db.h>

/* * * * * * * * * * * * * * * * * * *\
 *                                   *
 * Binary data file access functions *
 *            library                *
 *                                   *
\* * * * * * * * * * * * * * * * * * */

typedef struct
{  int      fd;
   int      first;
   int      recs;
   u_char * cache;
} seqread_t;

// Cached read structure (non-reenterable !)
seqread_t   sqr = {-1, 0, 0, NULL};


/* * * * * * * * * * * * * * * * * * * * * *\ 
 *  Open binary file for random read-write *
\* * * * * * * * * * * * * * * * * * * * * */

int db_open   (char * file)
{  int rc;

   rc = open(file, O_RDWR, 0600);
   if (rc < 0) syslog(LOG_ERR, "db_open(%s): %m", file);
   return rc; 
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\ 
 *  Open binary file for sequental (cached) read (-only) *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int dbs_open  (char * file)
{  int fd;

   fd = open(file, O_RDONLY, 0);
   if (fd < 0) syslog(LOG_ERR, "dbs_open(%s): %m", file);
   else
   {  if (sqr.fd < 0)
      {  bzero(&sqr, sizeof(sqr));
         sqr.fd = fd;
      } 
   }
   return fd;
} 


/* * * * * * * * * * * * * * * * *\ 
 *       Close binary file       *
\* * * * * * * * * * * * * * * * */

int db_close    (int fd)
{
   if (sqr.fd == fd)
   {  if (sqr.cache != NULL) free(sqr.cache); 
      bzero(&sqr, sizeof(sqr));
      sqr.fd = (-1);
   }
   return close(fd);
}


/* * * * * * * * * * * * * * * * *\ 
 *     Get file records count    *
\* * * * * * * * * * * * * * * * */

int db_reccount (int fd, int len)
{  struct stat fs;
   int         recs;
   int         rem;

   if (fstat(fd, &fs) != 0)
   {  syslog(LOG_ERR, "db_reccount(fstat): %m");
      return (-1);
   }
   if ((rem = fs.st_size % len) != 0)
   {  syslog(LOG_ERR, "db_reccount(): Warning: remandier = %d", rem);
   }
   recs = fs.st_size/len;

   return recs;
}


/* * * * * * * * * * * * * * * * *\ 
 *         Get record            *
\* * * * * * * * * * * * * * * * */

int db_get    (int fd, int rec, void * buf, int len)
{  int    recs;
   int    bytes;

/* get no of records */
    recs = db_reccount(fd, len);
    if (recs < 0) return IO_ERROR;

/* check size */
    if (recs <= rec) return NOT_FOUND;

    if (sqr.fd != fd)  // if random access
    {

/* RANDOM READ */

/* seek to record */
       if (lseek(fd, (off_t)rec * len, SEEK_SET)<0)
       {  syslog(LOG_ERR, "db_get(lseek): %m");
          return IO_ERROR;
       }

/* read record */    
       bytes = read(fd, buf, len);
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
    }
    else

/* CACHED READ */

    {
/* cache hittest & "read" */
       if (sqr.cache != NULL && sqr.recs != 0)
       {  if (rec >= sqr.first && rec < sqr.first + sqr.recs)
          {  memmove(buf, sqr.cache + ((rec - sqr.first) * len), len);
             return SUCCESS;  
          }
       }

/* allocate cache buffer (do not cache on failure) */
       if (sqr.cache == NULL)
       {  sqr.cache = calloc(1, len * 32768);
          if (sqr.cache == NULL)
          {  sqr.fd = (-1);                     // turn off caching
             return db_get(fd, rec, buf, len); 
          }
       }

/* lock file */
       flock(fd, LOCK_SH);

/* seek to target record */
       if (lseek(fd, (off_t)rec * len, SEEK_SET) < 0)
       {  syslog(LOG_ERR, "db_get(lseek): %m");
          return IO_ERROR;
       }

/* read data to cache */
       bytes = read(fd, sqr.cache, len * 32768);

/* unlock file */
       flock(fd, LOCK_UN);

/* check read responce */
       if (bytes < len)
       {  syslog(LOG_ERR, "db_get(read): Partial read %d (%d) at %d rec",
                 bytes, len, rec);
          return IO_ERROR;
       }
/* adjust cache data on read data */
       sqr.first = rec;
       sqr.recs  = bytes / len;

/* "read" first record from cache */
       memmove(buf, sqr.cache, len);
    }

    return SUCCESS;
}


/* * * * * * * * * * * * * * * * *\ 
 *          Put record           *
\* * * * * * * * * * * * * * * * */

int db_put    (int fd, int rec, void * data, int len)
{   int recs;
    int bytes;

/* get no of records */
    recs = db_reccount(fd, len);
    if (recs < 0) return IO_ERROR;

/* check size */
    if (recs <= rec) return NOT_FOUND;

/* seek to record */
    if (lseek(fd, (off_t)rec * len, SEEK_SET) < 0)
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


/* * * * * * * * * * * * * * * * *\ 
 *   Add (append) new record     *
 *     return record index       *
\* * * * * * * * * * * * * * * * */

int db_add    (int fd, void * data, int len)
{   int recs;
    int bytes;
   
/* get no of records */
    if ((recs = db_reccount(fd, len)) < 0) return IO_ERROR;

/* seek to next record (cutting off possible partial record) */
    if (lseek(fd, (off_t)recs * len, SEEK_SET) < 0)
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


/* * * * * * * * * * * * * * * * *\ 
 *       Lock file (shared)      *
\* * * * * * * * * * * * * * * * */

int db_shlock (int fd)
{   return flock(fd, LOCK_SH);  }


/* * * * * * * * * * * * * * * * *\ 
 *      Lock file (exlusive)     *
\* * * * * * * * * * * * * * * * */

int db_exlock (int fd)
{   return flock(fd, LOCK_EX);  }


/* * * * * * * * * * * * * * * * *\ 
 *         Unlock file           *
\* * * * * * * * * * * * * * * * */

int db_unlock (int fd)
{   return flock(fd, LOCK_UN);  }


/* * * * * * * * * * * * * * * * *\ 
 *       Sycnronize file         *
\* * * * * * * * * * * * * * * * */

int db_sync   (int fd)
{   return fsync(fd);           }


/* * * * * * * * * * * * * * * * *\ 
 *         Count CRC32           *
\* * * * * * * * * * * * * * * * */

int count_crc (void * data, int len)
{  uLong crc;

   crc = crc32(0L, Z_NULL, 0);
   crc = crc32(crc, data, len);

   return (int)crc;
}

