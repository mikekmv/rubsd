/* $Bee$ */
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
char       * host = "192.168.111.254";//BEE_ADDR; 
int          port = BEE_SERVICE;

int
main(argc, argv)
   int  argc;
   char **argv;
{
   int      acc;
   int      tag;
   int      balance;
   int      rc;
   char   * msg;
   char   * ptr;
   char   * str;
   char     linbuf[128];

   openlog("getacc", LOG_PID | LOG_NDELAY, LOG_DAEMON);

   if (argc < 2)
   {  printf("-51 Not enough argumets\n"); 
      return (-1);
   }

   rc = link_request(&lnk, host, port); 
   if (rc == -1)
   {  syslog(LOG_ERR, "Can't connect to billing service: %m"); 
      printf("-52 Can't connect to bee\n"); 
      exit(-1); 
   }

   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      printf("-53 No bee HELLO\n"); 
      exit(-1);
   }

   link_puts(&lnk, "machine");
   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      printf("-54 Bee error\n"); 
      exit(-1);
   }

   link_puts(&lnk, "acc inet name %s", argv[1]);
   rc = answait(&lnk, RET_STR, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_STR && rc < 400)
   {  if (rc == RET_SUCCESS) syslog(LOG_ERR, "Unexpected SUCCESS");
      if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      printf("-54 Bee error\n"); 
      exit(-1);
   }

   switch (rc)
   {  case RET_STR:
         ptr=msg;
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("-54 Bee error\n");
            break;
         }
         acc=strtol(str, NULL, 0);

         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("-54 Bee error\n");
            break;
         }
         tag=strtol(str, NULL, 0);

         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("-54 Bee error\n");
            break;
         }
         balance=strtod(str, NULL) * 100;

         printf("%d %d %d\n", acc, tag, balance);
         break; 
      case ERR_NOACC:
         printf("-1 Account not exist\n");
         exit(-1); 
      default:
         printf("-54 Bee error\n");
         exit(-1); 
   }

   link_puts(&lnk, "exit");
   link_close(&lnk);
   return 0;
}
