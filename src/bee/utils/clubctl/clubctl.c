/* $RuOBSD: getacc2.c,v 1.2 2001/09/12 05:03:21 tm Exp $ */
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
char       * host = "127.0.0.1";//BEE_ADDR; 
int          port = BEE_SERVICE;

#define MIN_NUMBER     1
#define MAX_NUMBER     8
#define NUMBER_SHIFT   0
char       * gatename = "station";
char         namebuf[16];
char         inbuf[80];

int main(int argc, char ** argv)
{
   int      acc;
   int      tag;
   int      balance;
   int      rc;
   char   * msg;
   char   * ptr;
   char   * str;
   char     linbuf[128];
   int      i;
   int      cmd;

   openlog("clubctl", LOG_PID | LOG_NDELAY, LOG_DAEMON);

   printf("Club control utility ver 0.1\n");

   do
   {  
// do print list
      printf("LIST:\n");
      rc = link_request(&lnk, host, port); 
      if (rc == -1)
      {  syslog(LOG_ERR, "Can't connect to billing service: %m"); 
         printf("ERROR - Can't connect to bee\n"); 
      }
      if (rc >= 0)      
      {  rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
            if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
            if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
            printf("ERROR - No bee HELLO\n"); 
         }
      }
      if (rc >= 0)
      {  link_puts(&lnk, "machine");
         rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
            if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
            if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
            printf("ERROR - Bee error\n"); 
         }
      }
      if (rc >= 0)
      {  
         for(i=MIN_NUMBER; i<=MAX_NUMBER; i++)
         {  sprintf(namebuf, "%s%d", gatename, i+NUMBER_SHIFT);  
            link_puts(&lnk, "acc adder name %s", namebuf);
         }
         for(i=MIN_NUMBER; i<=MAX_NUMBER; i++)
         {  rc = answait(&lnk, RET_STR, linbuf, sizeof(linbuf), &msg);
            if (rc != RET_STR && rc < 400)
            {  if (rc == RET_SUCCESS) syslog(LOG_ERR, "Unexpected SUCCESS");
               if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
               if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
               printf("ERROR - Bee error\n");
               break; 
            }
            switch (rc)
            {  case RET_STR:
                  ptr=msg;
                  str=next_token(&ptr, " \t");
                  if (str == NULL)
                  {  printf("station%02d - Bee error\n", i);
                     break;
                  }
                  acc=strtol(str, NULL, 0);

                  str=next_token(&ptr, " \t");
                  if (str == NULL)
                  {  printf("station%02d - Bee error\n", i);
                     break;
                  }
                  tag=strtol(str, NULL, 0);

                  str=next_token(&ptr, " \t");
                  if (str == NULL)
                  {  printf("ERROR - Bee error\n");
                     break;
                  }
                  balance=strtod(str, NULL) * 100;

                  printf("station%2d - %s\n", i, 
                        (tag & ATAG_UNLIMIT) ? "ON":"OFF");
                  rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
                  if (rc != RET_SUCCESS)
                  {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
                     if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
                     if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
                     printf("ERROR - Bee error\n"); 
                  }
                  break; 
               case ERR_NOACC:
                  printf("station%02d - Account not exist\n", i);
                  break;
               default:
                  printf("station%02d - Bee error\n", i);
            }
         }
         link_puts(&lnk, "exit");
         link_close(&lnk);
      }
// Do get command
      printf("\n> ");
      if (fgets(inbuf, sizeof(inbuf), stdin) == NULL) break;
      ptr = inbuf;
      str = next_token(&ptr, " \t\n");
      if (str == NULL) continue;
      cmd = -1;
      if (strcasecmp(str, "on") == 0) cmd = 1;
      if (strcasecmp(str, "off") == 0) cmd = 0;
      if (cmd < 0) continue;
      str = next_token(&ptr, " \t\n");
      if (str == NULL) 
      {  printf("ERROR - Station number expected\n");
         continue;
      }
      i = strtol(str, NULL, 10);
      if (i < MIN_NUMBER || i > MAX_NUMBER)
      {  printf("ERROR - Invalid station number\n");
         continue;
      }
// Do execute command
      rc = link_request(&lnk, host, port); 
      if (rc == -1)
      {  syslog(LOG_ERR, "Can't connect to billing service: %m"); 
         printf("ERROR - Can't connect to bee\n"); 
      }
      if (rc >= 0)      
      {  rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
            if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
            if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
            printf("ERROR - No bee HELLO\n"); 
         }
      }
      if (rc >= 0)
      {  link_puts(&lnk, "machine");
         rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
            if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
            if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
            printf("ERROR - Bee error\n"); 
         }
      }
      if (rc >= 0)
      {  sprintf(namebuf, "%s%d", gatename, i+NUMBER_SHIFT);  
         link_puts(&lnk, "%slimit adder name %s", cmd ? "un":"", namebuf);
         rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == RET_SUCCESS) syslog(LOG_ERR, "Unexpected SUCCESS");
            if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
            if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
            printf("ERROR - Bee error\n");
         }
      }
      if (rc >= 0)
      {  link_puts(&lnk, "update");
         rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
         if (rc != RET_SUCCESS)
         {  if (rc == RET_SUCCESS) syslog(LOG_ERR, "Unexpected SUCCESS");
            if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
            if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
            printf("ERROR - Bee error\n");
         }
      }
      link_puts(&lnk, "exit");
      link_close(&lnk);
   } while (1);   

   return 0;
}
