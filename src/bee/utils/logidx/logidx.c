#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


#include <bee.h>
#include <db.h>
#include <res.h>
#include <logidx.h>

char      * logname = NULL;
logbase_t   Logbase;
int         recs;

char      * idxname = NULL;
char        idxtemp[512];

logrec_t    logrec;
idxhead_t   idxhead = {  IDXMARKER, 0  };

int main(int argc, char ** argv)
{  int          rc;
   int          fd = (-1);
   struct tm    stm;
   time_t       next;
   long long    ind = 1;

// Load configuration
   rc = conf_load(NULL);
   if (rc < 0)
   {  fprintf(stderr, "ERROR - Config loading failure\n");
      exit(-1);
   }

// Set defaults to config values
   logname = conf_logfile;
   idxname = conf_logindex;

// temporary filename template create
   strlcpy(idxtemp, idxname, sizeof(idxtemp));
   strlcat(idxtemp, ".XXXXXXXX", sizeof(idxtemp));

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
   {  syslog(LOG_ERR, "log_baseopen(%s) : Error. ABORT", logname);
      return (-1);
   }

// Open output (bee log index)
   fd = mkstemp(idxtemp);
   if (fd < 0)
   {  syslog(LOG_ERR, "mkstemp(%s): %m. ABORT", idxtemp);
      return (-1);
   }

   while(1) // fictive cycle
   {
   // Create index header
   //   get first record
      rc = logi_get(&Logbase, 0, &logrec);
      if (rc == IO_ERROR || rc == NOT_FOUND)
      {  syslog(LOG_ERR, "log_get(first): Error. ABORT");
         close(fd);
         fd = (-1);
         break;
      }
   //   count day origin
      if (localtime_r(&(logrec.time), &stm) == NULL)
      {  syslog(LOG_ERR, "localtime_r(): Error, aborting");
         close(fd);
         fd = (-1);
         break;
      }
      stm.tm_sec  = 0;
      stm.tm_min  = 0;
      stm.tm_hour = 0;
      idxhead.first = mktime(&stm);

      if (idxhead.first < 0)
      {  syslog(LOG_ERR, "mktime(): Error. ABORT");
         close(fd);
         fd = (-1);
         break;
      }
   //   write header
      rc = write(fd, &idxhead, sizeof(idxhead));
      if (rc < 0)
      {  syslog(LOG_ERR, "write(idx header): %m. ABORT");
         close(fd);
         fd = (-1);
         break;
      }
      if (rc < sizeof(idxhead))
      {  syslog(LOG_ERR, "write(idx header): Partial write. ABORT");
         close(fd);
         fd = (-1);
         break;
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
         {  syslog(LOG_ERR, "log_get(%lld): Error (%d). ABORT", ind, rc);
            close(fd);
            fd = (-1);
            break; 
         }
         rc = sizeof(ind);
         while (((int)next - (int)(logrec.time)) <= 0)
         {  rc = write(fd, &ind, sizeof(ind));
            if (rc < 0)
            {  syslog(LOG_ERR, "write(index): %m. ABORT");
               close(fd);
               fd = (-1);
               break;
            }
            if (rc < sizeof(ind))
            {  syslog(LOG_ERR, "write(index): Partial write. ABORT");
               close(fd);
               fd = (-1);
               break;
            }
            next += ONE_DAY; 
         }
         if (rc < sizeof(ind) || fd < 0) break;
         ind++; 
      }

      break; // end of fictive cycle
   } // fictive while(1)

// Close files
   log_baseclose(&Logbase);
  
   if (fd < 0) unlink(idxtemp);
   else 
   {  close(fd);

   // Rename & chmod temporary to index
      rc = rename(idxtemp, idxname);
      if (rc < 0)
      {  syslog(LOG_ERR, "rename(%s -> %s): %m. ABORT", idxtemp, idxname);
         unlink(idxtemp);
      }

      rc = chmod(idxname, 0644);
      if (rc < 0) syslog(LOG_ERR, "chmod(%s, 0644): %m (ignored)", idxname);
   } 
   
   return 0;
}
