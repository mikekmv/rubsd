#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"


char    flogname[] = "/var/log/payman";

FILE  * flog;

int log_open  ()
{
   flog = fopen(flogname, "a");
   if (flog == NULL)
   {  syslog(LOG_ERR, "log_open(fopen): %m");
      return (-1);
   }

   return 0;
}


int log_close ()
{  int  rc;
  
   if (flog == NULL) return (-1);
   
   do 
   {   rc = fclose(flog);
   } while (rc == EOF && errno == EINTR);
   
   if (rc == EOF)
   {  syslog(LOG_ERR, "log_close(fclose): %m");
      return (-1);
   }

   return 0;
}

int log_write (char * format, ...)
{  time_t     utc;
   struct tm  stm;

   va_list    valist;

   char     * buf     = NULL;
   char     * timebuf = NULL;

   utc = time(NULL);

   if (localtime_r(&utc, &stm) == NULL)
   {  syslog(LOG_ERR, "log_write(localtime_r): Error");
      return (-1);
   }
   
   asprintf(&timebuf, "%02d/%02d/%02d %02d:%02d.%02d (%lu)",
           stm.tm_mday, stm.tm_mon + 1, stm.tm_year%100,
           stm.tm_hour, stm.tm_min, stm.tm_sec,
           (unsigned long)getpid());

   if (timebuf == NULL)
      syslog(LOG_ERR, "log_write(asprintf(time)): NULL string");

   if (format != NULL) 
   {  va_start(valist, format);
      vasprintf(&buf, format, valist);
      va_end(valist);

      if (buf == NULL)
         syslog(LOG_ERR, "log_write(vasprintf(msg)): NULL string");
   }

   flock(fileno(flog), LOCK_EX);

   fprintf(flog, "%s %s\n", 
             timebuf == NULL ? "(unknown time)" : timebuf,
             buf == NULL     ? "" : buf);
   fflush(flog);

   flock(fileno(flog), LOCK_UN);

   if (timebuf != NULL) free(timebuf);
   if (buf != NULL)     free(buf);

   return 0;
}

