#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include <bee.h>
#include <db.h>
#include <res.h>

#include "logidx.h"

char      * logname = NULL;
logbase_t   Logbase;
int         recs;

char      * idxname = NULL;

logrec_t    logrec;
idxhead_t   idxhead = {  IDXMARKER, 0  };

int main(int argc, char ** argv)
{  int          rc;
   int          fd;
   struct tm    stm;
   time_t       next;
   u_int        ind = 1;

// Load configuration
   rc = conf_load(NULL);
   if (rc < 0)
   {  fprintf(stderr, "ERROR - Config loading failure\n");
      exit(-1);
   }

// Set defaults to config values
   logname = conf_logfile;
   idxname = conf_logindex;


#define PARAMS "f:o:"

   while ((rc = getopt(argc, argv, PARAMS)) != -1)
   {  switch (rc)
      {
         case 'f':
            logname = optarg;
            break;

         case 'o':
            idxname = optarg;
            break;

         default:
            printf("unexpected switch \"%c\"\n", rc);
      }
   }

// Open source (bee log)
   rc = log_baseopen_sr(&Logbase, logname);
   if (rc < 0)
   {  syslog(LOG_ERR, "log_baseopen(%s) : Error", logname);
      return (-1);
   }

// Open output (bee log index)
   fd = open(idxname, O_WRONLY | O_TRUNC | O_CREAT | O_EXLOCK, 0644);
   if (fd < 0)
   {  syslog(LOG_ERR, "open(%s): %m", idxname);
      exit(-1);
   }

// Create index header
//   get first record
   rc = logi_get(&Logbase, 0, &logrec);
   if (rc == IO_ERROR || rc == NOT_FOUND)
   {  syslog(LOG_ERR, "log_get(first): Error");
      exit(-1);
   }
//   count day origin
   if (localtime_r(&(logrec.time), &stm) == NULL)
   {  syslog(LOG_ERR, "localtime_r(): Error, aborting");
      exit(-1);
   }
   stm.tm_sec  = 0;
   stm.tm_min  = 0;
   stm.tm_hour = 0;
   idxhead.first = mktime(&stm);

   if (idxhead.first < 0)
   {  syslog(LOG_ERR, "mktime(): Error, aborting");
      exit(-1);
   }
//   write header
   rc = write(fd, &idxhead, sizeof(idxhead));
   if (rc < 0)
   {  syslog(LOG_ERR, "write(idx header): %m");
      exit(-1);
   }
   if (rc < sizeof(idxhead))
   {  syslog(LOG_ERR, "write(idx header): Partial write");
      exit(-1);
   }

#define ONE_DAY (3600*24)
      
// Main cycle
   next = idxhead.first + ONE_DAY;

   while((rc = logi_get(&Logbase, ind, &logrec)) != NOT_FOUND)
   {  if (rc == ACC_BROKEN)  // skip invalid entries
      {  ind++;
         continue;
      }
      if (rc < 0) 
      {  syslog(LOG_ERR, "log_get(%d): Error (%d), stopped", ind, rc);
         break; 
      }
      rc = sizeof(ind);
      while (((int)next - (int)(logrec.time)) <= 0)
      {  rc = write(fd, &ind, sizeof(ind));
         if (rc < 0)
         {  syslog(LOG_ERR, "write(index): %m, stopped");
            break;
         }
         if (rc < sizeof(ind))
         {  syslog(LOG_ERR, "write(index): Partial write, stopped");
            break;
         }
         next += ONE_DAY; 
      }
      if (rc < sizeof(ind)) break;
      ind++; 
   }

// Close files
   close(fd);
   log_baseclose(&Logbase);

   return 0;
}
