/* $Bee$ */

#include <sys/cdefs.h>
#include <syslog.h>
#include <varargs.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <bee.h>
#include <ipc.h>
#include <db.h>
#include <core.h>
#include <command.h>
#include <res.h>
#include <links.h>

/*
   Init billing:
   >> Parse command line
   >> Configuring (if any)
   >> Open Account DB
   >> Open LogDB
   >> Check Accounts & enable resources
   >> Open IPC channel
   >> Hear for modules
   >> Process Queries
 */

accbase_t Accbase;
logbase_t Logbase;

int       HumanRead=1;
int       MachineRead=1;
int	  NeedUpdate=0;

char   sbuf[128];
char   outbuf[256];

link_t   internal_link={0,0,0};
link_t * ld=&internal_link;
int      OwnService=BEE_SERVICE;
char   * accbase_name= "/var/bee/account.dat";
char   * linkfile_name="/var/bee/reslinks.cfg";
char   * logbase_name= "/var/bee/beelog.dat";

char   * ApplyScript="/usr/local/bin/beeapply.sh";

int db_reccount(int fd, int len);

int main(int argc, char ** argv)
{  int             c;
   int             rc;
//   int             i;
   int             fRun=0;
   int             fDaemon=0;
   int             fUpdate=0;

   openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);

/* TODO
 A  1. Redefine service port
 u  2. Update access at start
 d  3. Run Daemon
 c  4. Run console
*/

   while ((c = getopt(argc, argv, OPTS)) != -1)
   {  switch (c)
      {
      case 'A':
         OwnService=strtol(optarg, NULL, 0);
         break;
      case 'c':
         ld->fStdio=1;
         fRun=1;
         break;
      case 'd':
         fDaemon=1;
         fRun=1;
         break;
      case 'u':
         fUpdate=1;
         break;
      case 'h':
      case '?':
	 usage(0);
      default:
	 usage(-1);
      }
   }

   if (fDaemon==1 && ld->fStdio==1)
   {  fprintf(stderr, "Can't daemonize in console mode\n");
      fDaemon=0;
   }

// Load linktable (check file integrity)
   rc=reslinks_load(linkfile_name);
   if (rc != SUCCESS)
   {  syslog(LOG_ERR, "Can't load links file");
      exit (-1);
   }
// Open database
   rc=acc_baseopen(&Accbase, accbase_name);
   if (rc<0)
   {  syslog(LOG_ERR, "Can't open account database");
      exit (-1);
   }   
   rc=log_baseopen(&Logbase, logbase_name);
   if (rc<0)
   {  syslog(LOG_ERR, "Can't open log database");
      acc_baseclose(&Accbase);
      exit (-1);
   }   
// Enable resources
   if (fUpdate) access_update(); 
// Start server
   if (fRun)
   {  rc=link_wait(ld, OwnService);
      if (rc != -1)
      {  cmd_out(RET_COMMENT, "Billing ver 0.0.1.0");
         cmd_out(RET_COMMENT, "loading resource links ...");
// ReLoad linktable
         rc=reslinks_load(linkfile_name);
         if (rc != SUCCESS)
         {  cmd_out(ERR_IOERROR, "failure: %s", strerror(errno));
            exit (-1);
         }
         cmd_out(RET_SUCCESS, "Ready");
         while(1)
         {  rc=link_gets(ld, sbuf, sizeof(sbuf));
            if (rc==LINK_DOWN) break;
            if (rc==SUCCESS) 
            {  rc=cmd_exec(sbuf);
            if (rc==LINK_DOWN || rc==CMD_EXIT) break;
            }
         }
      }
   }
   acc_baseclose(&Accbase);
   log_baseclose(&Logbase);
   return 0;
}

void usage(int code)
{  syslog(LOG_ERR, "usage: %s %s", __progname, OPTS);
   closelog();
   exit(code);
}

int access_update()
{  FILE * f[16];
   char   filename[64];
   int    accs;
   int    i;
   int    ind;
   int    lockfd;
   acc_t  acc;
   int    rc;
   
   lockfd=open("/var/bee/lockfile", O_CREAT | O_EXLOCK);

   for (i=0; i<resourcecnt; i++)
   {  snprintf(filename, sizeof(filename), 
               "/var/bee/allowed.%s", resource[i].name);
      f[i]=fopen(filename, "w");
      if (f[i]==NULL) syslog(LOG_ERR, "fopen(%s): %m", filename);
   }
   accs=acc_reccount(&Accbase);
   for (i=0; i<accs; i++)
   {  
      rc=acc_get(&Accbase, i, &acc);
      if (rc == SUCCESS || rc == ACC_UNLIMIT)
      {  ind=-1;
         while (lookup_accno(i, &ind) >= 0)
         {  if (f[linktab[ind].res_id] != NULL)
               fprintf(f[linktab[ind].res_id], "%s\n", 
                           linktab[ind].username);
         }
      }
   }   
   for (i=0; i<resourcecnt; i++) fclose(f[i]); 

   for (i=0; i<resourcecnt; i++)
   {  if (resource[i].ruler_cmd != NULL)
      {  rc=system(resource[i].ruler_cmd);
         switch (rc)
         {  case (-1):
              syslog(LOG_ERR, "system(%s): %m", resource[i].ruler_cmd);
              break;
            case 127:
              syslog(LOG_ERR, "system(%s): sh(1) fails", resource[i].ruler_cmd);
              break;
            default:
              if (rc != NULL) 
                 syslog(LOG_ERR, "%s ret=%d", resource[i].ruler_cmd, rc);
         }
      } 
   }
   rc=system(ApplyScript);
   switch (rc)
   {  case (-1):
        syslog(LOG_ERR, "system(%s): %m", ApplyScript);
        break;
      case 127:
        syslog(LOG_ERR, "system(%s): sh(1) fails", ApplyScript);
        break;
      default:
        if (rc != NULL) 
          syslog(LOG_ERR, "%s ret=%d", ApplyScript, rc);
   }
   close(lockfd);
   return 0;
}

