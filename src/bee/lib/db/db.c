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
{  int         fd;
   long long   first;
   long long   recs;
   u_char    * cache;
} seqread_t;

// Cached read structure (non-reenterable !)
seqread_t   sqr = {-1, 0, 0, NULL};


/* * * * * * * * * * * * * * * * * * * * * *\ 
 *  Open binary file for random read-write *
\* * * * * * * * * * * * * * * * * * * * * */

int db_open   (char * filespec)
{  int rc;

   rc = open(filespec, O_RDWR, 0600);
   if (rc < 0) syslog(LOG_ERR, "db_open(%s): %m", filespec);

   return rc;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\ 
 *  Open binary file for sequental (cached) read (-only) *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int dbs_open  (char * filespec)
{  int fd;

   fd = open(filespec, O_RDONLY, 0);
   if (fd < 0) syslog(LOG_ERR, "dbs_open(%s): %m", filespec);
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

long long db_reccount (int fd, long long recsize)
{  struct stat fs;
   long long   recs;
   long long   rem;

   if (fstat(fd, &fs) != 0)
   {  syslog(LOG_ERR, "db_reccount(fstat): %m");
      return (-1);
   }

   if ((rem = fs.st_size % recsize) != 0)
   {  syslog(LOG_ERR, "db_reccount(): Warning: remandier = %lld", rem);
   }
   recs = fs.st_size / recsize;

   return recs;
}


/* * * * * * * * * * * * * * * * *\ 
 *         Get record            *
\* * * * * * * * * * * * * * * * */

int db_get    (int  fd, long long  rec, void *  buf, long long  recsize)
{  long long  recs;
   long long  bytes;

/* get no of records */
    recs = db_reccount(fd, recsize);
    if (recs < 0) return IO_ERROR;

/* check size */
    if (recs <= rec) return NOT_FOUND;

    if (sqr.fd != fd)  // if random access
    {

/* RANDOM READ */

/* seek to record */
       if (lseek(fd, (off_t)rec * recsize, SEEK_SET)<0)
       {  syslog(LOG_ERR, "db_get(lseek): %m");
          return IO_ERROR;
       }

/* read record */    
       bytes = read(fd, buf, recsize);
       if (bytes < 0)
       {  syslog(LOG_ERR, "db_get(read): %m");
          return IO_ERROR;
       }

/* check for partial read */    
       if (bytes < recsize)
       {  syslog(LOG_ERR, "db_get(read): Partial read %lld (%lld) at %lld rec",
                 bytes, recsize, rec);
          return IO_ERROR;
       }
    }
    else

/* CACHED READ */

    {
/* cache hittest & "read" */
       if (sqr.cache != NULL && sqr.recs != 0)
       {  if (rec >= sqr.first && rec < sqr.first + sqr.recs)
          {  memmove(buf, sqr.cache + ((rec - sqr.first) * recsize), recsize);
             return SUCCESS;
          }
       }

/* allocate cache buffer (do not cache on failure) */
       if (sqr.cache == NULL)
       {  sqr.cache = calloc(1, recsize * 32768);
          if (sqr.cache == NULL)
          {  sqr.fd = (-1);                     // turn off caching
             return db_get(fd, rec, buf, recsize); 
          }
       }

/* lock file */
       flock(fd, LOCK_SH);

/* seek to target record */
       if (lseek(fd, (off_t)rec * recsize, SEEK_SET) < 0)
       {  syslog(LOG_ERR, "db_get(lseek): %m");
          return IO_ERROR;
       }

/* read data to cache */
       bytes = read(fd, sqr.cache, recsize * 32768);

/* unlock file */
       flock(fd, LOCK_UN);

/* check read responce */
       if (bytes < recsize)
       {  syslog(LOG_ERR, "db_get(read): Partial read %lld (%lld) at %lld rec",
                 bytes, recsize, rec);
          return IO_ERROR;
       }
/* adjust cache data on read data */
       sqr.first = rec;
       sqr.recs  = bytes / recsize;

/* "read" first record from cache */
       memmove(buf, sqr.cache, recsize);
    }

    return SUCCESS;
}


/* * * * * * * * * * * * * * * * *\ 
 *          Put record           *
\* * * * * * * * * * * * * * * * */

int db_put    (int  fd, long long  rec, void * data, long long  recsize)
{   long long recs;
    long long bytes;

/* get no of records */
    recs = db_reccount(fd, recsize);
    if (recs < 0) return IO_ERROR;

/* check size */
    if (recs <= rec) return NOT_FOUND;

/* seek to record */
    if (lseek(fd, (off_t)rec * recsize, SEEK_SET) < 0)
    {  syslog(LOG_ERR, "db_put(lseek): %m");
       return IO_ERROR;
    }

/* write record */    
    bytes = write(fd, data, recsize);
    if (bytes < 0)
    {  syslog(LOG_ERR, "db_put(write): %m");
       return IO_ERROR;
    }

/* check for partial write */    
    if (bytes < recsize)
    {  syslog(LOG_ERR, "db_put(read): Partial write %lld (%lld) at %lld rec",
       bytes, recsize, rec);
       return IO_ERROR;
    }

    return SUCCESS;
}


/* * * * * * * * * * * * * * * * *\ 
 *   Add (append) new record     *
 *     return record index       *
\* * * * * * * * * * * * * * * * */

long long  db_add    (int fd, void * data, long long  recsize)
{   long long  recs;
    long long  bytes;
   
/* get no of records */
    if ((recs = db_reccount(fd, recsize)) < 0) return IO_ERROR;

/* seek to next record (cutting off possible partial record) */
    if (lseek(fd, (off_t)recs * recsize, SEEK_SET) < 0)
    {  syslog(LOG_ERR, "db_add(lseek): %m");
       return IO_ERROR;
    }

/* write record */    
    bytes = write(fd, data, recsize);
    if (bytes < 0)
    {  syslog(LOG_ERR, "db_add(write): %m");
       return IO_ERROR;
    }

/* check for partial write */    
    if (bytes < recsize)
    {  syslog(LOG_ERR, "db_add(write): Partial write %lld (%lld)",
              bytes, recsize);
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

int count_crc (void * data, long long len)
{  uLong crc;

   crc = crc32(0L, Z_NULL, 0);
   crc = crc32(crc, data, len);

   return (int)crc;
}

