/* $RuOBSD: core.c,v 1.12 2006/12/30 09:43:02 shadow Exp $ */

#include <sys/cdefs.h>
#include <syslog.h>
#include <varargs.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <version.h>
#include <bee.h>
#include <ipc.h>
#include <db.h>
#include <core.h>
#include <command.h>
#include <res.h>
#include <links.h>
#include <timer.h>

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
char   * logbase_name= "/var/bee/beelog.dat";

char   * ApplyScript="/usr/local/bin/beeapply.sh";
char   * IntraScript= "/usr/local/sbin/intractl.sh /etc/bee/intra.conf"
                      " > /dev/null";

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
   rc = reslinks_load(LOCK_SH);
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
// Close database
   acc_baseclose(&Accbase);
   log_baseclose(&Logbase);
// Start server
   if (fRun)
   {  if (fDaemon)
      {  rc=daemon(0,0);
         if (rc != SUCCESS)
         {  syslog(LOG_ERR, "Can't daemonize, closing");
            exit(-1);
         }
         setproctitle("(daemon)");
      }
      if (setenv("HOME", "/root", 0) == (-1)) syslog(LOG_ERR, "setenv(): %m");
      rc = link_wait(ld, OwnService);
      if (rc != -1)
      {  
         if (ld->fStdio == 0) setproctitle("(fork)");
         else setproctitle("(console)");
         cmd_out(RET_COMMENT, "Billing ver %d.%d.%d.%d", VER_VER, VER_SUBVER, VER_REV, VER_SUBREV);
         cmd_out(RET_COMMENT, "loading resource links ...");
// ReLoad linktable
         rc = reslinks_load(LOCK_SH);
         if (rc != SUCCESS)
         {  cmd_out(ERR_IOERROR, "failure: %s", strerror(errno));
            exit (-1);
         }
// Open database
         cmd_out(RET_COMMENT, "opening accounts ...");
         rc = acc_baseopen(&Accbase, accbase_name);
         if (rc < 0)
         {  syslog(LOG_ERR, "Can't reopen account database");
            cmd_out(ERR_IOERROR, "failure: %s", strerror(errno));
            exit (-1);
         }
         cmd_out(RET_COMMENT, "opening log ...");
         rc = log_baseopen(&Logbase, logbase_name);
         if (rc < 0)
         {  syslog(LOG_ERR, "Can't reopen log database");
            cmd_out(ERR_IOERROR, "failure: %s", strerror(errno));
            acc_baseclose(&Accbase);
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
{  FILE *  f[resourcecnt];
   FILE * f2[resourcecnt];
   FILE * fil;
   char   filename[64];
   int    accs;
   int    i;
   int    ind;
   int    lockfd;
   acc_t  acc;
   int    rc;
   timeval_t locktimer;
   
// open lockfile
   lockfd = open("/var/bee/lockfile", O_CREAT);
   if (lockfd == NULL)
   {  syslog(LOG_ERR, "access_update(open(lockfile)): %m");
      return (-1);
   }

// try to lock lockfile (w/timeout)
   tm_sets(&locktimer, 3);          // 3 seconds timeout

   rc = (-1);                       // for safety 
   while(tm_state(&locktimer))
   {  rc = flock(lockfd, LOCK_EX | LOCK_NB);
      if (rc >= 0) break;
      if (errno != EWOULDBLOCK) break;
      usleep(10000);
   } 
   
// fail on error
   if (rc < 0)
   {  syslog(LOG_ERR, "access_update(flock): %m");
      close(lockfd);
      return (-1);
   }

   for (i=0; i<resourcecnt; i++)
   {  snprintf(filename, sizeof(filename), 
               "/var/bee/allowed.%s", resource[i].name);
      f[i]=fopen(filename, "w");
      if (f[i]==NULL) syslog(LOG_ERR, "fopen(%s): %m", filename);
      snprintf(filename, sizeof(filename), 
               "/var/bee/disallowed.%s", resource[i].name);
      f2[i]=fopen(filename, "w");
      if (f2[i]==NULL) syslog(LOG_ERR, "fopen(%s): %m", filename);
   }
   accs = acc_reccount(&Accbase);

   for (i=0; i<accs; i++)
   {  rc = acc_get(&Accbase, i, &acc);
      if (rc == ACC_UNLIMIT) rc = SUCCESS;
      else
      {  if (rc == SUCCESS || rc == NEGATIVE) rc = accs_state(&acc);
      }
      ind = -1;
      while (lookup_accno(i, &ind) >= 0)
      {  if (f[linktab[ind].res_id] != NULL)
         {  if (rc == SUCCESS && linktab[ind].allow) 
                 fil= f[linktab[ind].res_id];
            else fil=f2[linktab[ind].res_id];
            fprintf(fil, "%s\n", linktab[ind].username);
         }
      }
   }   

   for (i=0; i<resourcecnt; i++) 
   {  fclose( f[i]);
      fclose(f2[i]);
   } 

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

   rc = system(ApplyScript);

   switch (rc)
   {  case (-1):
        syslog(LOG_ERR, "system(%s): %m", ApplyScript);
        break;
      case 127:
        syslog(LOG_ERR, "system(%s): sh(1) fails", ApplyScript);
        break;
      default:
        if (rc != NULL) 
          syslog(LOG_ERR, "%s ret = %d", ApplyScript, rc);
   }


   flock(lockfd, LOCK_UN);
   close(lockfd);

   return 0;
}

#define LogStep 3600

int acc_transaction (accbase_t * base, logbase_t * logbase, int accno, is_data_t * isdata, int arg)
{  int      rc;
   acc_t    acc;
   logrec_t logrec;
   logrec_t oldrec;
   int      i;
   int      recs;
   money_t  sum = 0;

   memset(&logrec, 0, sizeof(logrec));

   rc = acc_baselock(base);
   if (rc >= 0)
   {
// log fixed fields
      logrec.time  = time(NULL);   // stub
      logrec.accno = accno;
      if (isdata != NULL) logrec.isdata = *isdata;
      else  logrec.isdata.res_id = (-1);
// get account
      for (i=0; i<3; i++)
         if ((rc = acci_get(base, accno, &acc)) != IO_ERROR) break;
      logrec.serrno = rc;
// if account is not broken, store balance
      if (rc >= 0 || rc <= ACC_FROZEN) logrec.balance = acc.balance;

// Count transaction sum
   if (arg == (-1)) 
      sum = resource[isdata->res_id].count(isdata, &acc); 
   else
      sum = - ((money_t)isdata->value * (((money_t)arg)/100) / 1048576);

   logrec.sum   = sum;

// if account in valid (not frozen) count new balance
      if (rc == SUCCESS || rc == NEGATIVE || rc == ACC_OFF) acc.balance += sum;
// if account is valid - write account back
      if (rc >= 0 || rc == ACC_OFF)
      {  for (i=0; i<3; i++)
            if ((rc = acci_put(base, accno, &acc)) != IO_ERROR) break;
// if write insuccess - log error
         if (rc < 0) logrec.serrno = rc;
      }
      if ((rc = log_baselock(logbase)) == SUCCESS)
      {  recs = log_reccount(logbase);
         rc = (-1);
         for (i=recs-1; i>=0; i--)
         {  if ((rc = logi_get(logbase, i, &oldrec)) == SUCCESS)
            {  if (logrec.time - oldrec.time < LogStep)
               {  if (logrec.accno == oldrec.accno                 &&
                      logrec.serrno == oldrec.serrno                 &&
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

   return accs_state(&acc);
}

typedef struct
{  int      reserv;
   money_t  min;
} limit_t;

limit_t    limits[]=
{
  { 0,     0.00 }, // default limit
  {-1, -1       }  // (terminator)
};

int accs_state(acc_t * acc)
{  int limit = 0; // default limit
   int i = 0;

   for (i = 0; limits[i].reserv >= 0; i++)
   {  if (acc->reserv[0] == limits[i].reserv)
      {  limit = i;
         break;
      } 
   }

   return (acc->balance < (limits[limit].min + 0.01));
}

