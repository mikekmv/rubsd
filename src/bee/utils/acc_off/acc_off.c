/* $RuOBSD$ */
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>

#include <bee.h>
#include <ipc.h>
#include <db.h>

link_t	     lnk;
char       * host = BEE_ADDR; 
int          port = BEE_SERVICE;

int
main(argc, argv)
   int  argc;
   char **argv;
{
   int      rc;
   char   * msg;
   char     linbuf[128];

   openlog("getacc", LOG_PID | LOG_NDELAY, LOG_DAEMON);

   if (argc < 2)
   {  syslog(LOG_ERR, "argument requried");
      return (-1);
   }

   rc = link_request(&lnk, host, port); 
   if (rc == -1)
   {  syslog(LOG_ERR, "Can't connect to billing service: %m"); 
      exit(-1); 
   }

   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      exit(-1);
   }

   link_puts(&lnk, "machine");
   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      exit(-1);
   }

   link_puts(&lnk, "off inet name %s", argv[1]);
   rc = answait(&lnk, RET_STR, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      exit(-1);
   }

   link_puts(&lnk, "exit");
   link_close(&lnk);
   return 0;
}
