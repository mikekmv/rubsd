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
char       * host = BEE_ADDR; 
int          port = BEE_SERVICE;

int
main(argc, argv)
   int  argc;
   char **argv;
{
   int      tag;
   int      balance;
   int      rc;
   char   * msg;
   char   * ptr;
   char   * str;
   int      colon;
   char     linbuf[128];

   openlog("getacc", LOG_PID | LOG_NDELAY, LOG_DAEMON);

   if (argc < 2)
   {  printf("Cведения о счете недоступны: ошибка скрипта\n"); 
      return (-1);
   }

   rc = link_request(&lnk, host, port); 
   if (rc == -1)
   {  syslog(LOG_ERR, "Can't connect to billing service: %m"); 
      printf("Cведения о счете недоступны: билинг не запущен\n"); 
      exit(-1); 
   }

   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      printf("Cведения о счете недоступны: ошибка билинга\n"); 
      exit(-1);
   }

   link_puts(&lnk, "machine");
   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      printf("Cведения о счете недоступны: ошибка билинга\n"); 
      exit(-1);
   }

   link_puts(&lnk, "acc inet name %s", argv[1]);
   rc = answait(&lnk, RET_STR, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_STR && rc < 400)
   {  if (rc == RET_SUCCESS) syslog(LOG_ERR, "Unexpected SUCCESS");
      if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      printf("Cведения о счете недоступны: ошибка билинга\n"); 
      exit(-1);
   }
   switch (rc)
   {  case RET_STR:
         ptr=msg;
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("Cведения о счете недоступны: ошибка билинга\n");
            break;
         }
         printf("Cчет #%04ld ", strtol(str, NULL, 0));
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("ошибка билинга\n");
            break;
         }
         tag=strtol(str, NULL, 0);
         colon=0;
         if (tag != 0) printf("(");
         if (tag & ATAG_DELETED) 
         {  printf("cчет закрыт)\n");
            break;
         }
         if (tag & ATAG_BROKEN)
         {  printf("cчет поврежден)\n");
            break;
         }
         if (tag & ATAG_UNLIMIT)
         {  if (colon) printf(", ");
            printf("неограниченный доступ");
            colon=1;
         }
         if (tag & ATAG_FROZEN)
         {  if (colon) printf(", ");
            printf("заморожен");
            colon=1;
         }
         if (tag & ATAG_OFF)
         {  if (colon) printf(", ");
            printf("приостановлен");
            colon=1;
         }
         if (tag != 0) printf(") ");
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("ошибка билинга\n");
            break;
         }
         balance=strtod(str, NULL) * 100;
         if (balance < 0) printf("долг: ");
         else printf("остаток: ");
         balance=abs(balance);
         printf("%d руб. %d коп.\n", balance/100, balance%100);
         break; 
      case ERR_NOACC:
         printf("Вашего счета не существует\n");
         exit(-1); 
      default:
         printf("Cведения о счете недоступны: ошибка билинга\n");
         exit(-1); 
   }

   link_puts(&lnk, "exit");
   link_close(&lnk);
   return 0;
}
