/* $RuOBSD: getacc.c,v 1.2 2001/09/12 05:03:21 tm Exp $ */
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
   {  printf("<H2>Данные недоступны</H2><br>ошибка скрипта"); 
      return (-1);
   }

   rc = link_request(&lnk, host, port); 
   if (rc == -1)
   {  syslog(LOG_ERR, "Can't connect to billing service: %m"); 
      printf("<H2>Данные недоступны</H2><br>сервис не найден"); 
      exit(-1); 
   }

   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      printf("<H2>Данные недоступны</H2><BR>ошибка сервиса"); 
      exit(-1);
   }

   link_puts(&lnk, "machine");
   rc = answait(&lnk, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      if (rc >= 400) syslog(LOG_ERR, "Billing error: %s", msg);
      printf("<H2>Данные недоступны</H2><BR>ошибка сервиса"); 
      exit(-1);
   }

   link_puts(&lnk, "acc inet name %s", argv[1]);
   rc = answait(&lnk, RET_STR, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_STR && rc < 400)
   {  if (rc == RET_SUCCESS) syslog(LOG_ERR, "Unexpected SUCCESS");
      if (rc == LINK_DOWN) syslog(LOG_ERR,"Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "Link error: %m");
      printf("<H2>Данные недоступны</H2><BR>ошибка сервиса"); 
      exit(-1);
   }
   switch (rc)
   {  case RET_STR:
         ptr=msg;
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("<H2>Данные недоступны</H2><BR>ошибка сервиса");
            break;
         }
         printf("Cчет #%04ld<BR>", strtol(str, NULL, 0));
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("<H2>Данные недоступны</H2><BR>ошибка сервиса");
            break;
         }
         tag=strtol(str, NULL, 0);
         colon=0;
         if (tag & ATAG_DELETED) 
         {  printf("<H2>cчет закрыт</H2>\n");
            break;
         }
         if (tag & ATAG_BROKEN)
         {  printf("<H2>cчет поврежден</H2>\n");
            break;
         }
         if (tag & ATAG_FROZEN)
         {  printf("<H2>счет заморожен</H2><BR>");
            colon=1;
         }
         if (tag & ATAG_OFF)
         {  if (! colon) printf("<H2>");
            printf("счет приостановлен");
            if (! colon) 
            {  printf("</H2>");
               colon = 1;
            }
            printf("<BR>");
         }
         if (tag & ATAG_UNLIMIT)
         {  if (! colon) printf("<H2>");
            printf("нелимитированный доступ");
            if (! colon) 
            {  printf("</H2>");
               colon = 1;
            }
            printf("<BR>");
         }
         str=next_token(&ptr, " \t");
         if (str == NULL)
         {  printf("ошибка билинга");
            break;
         }
         balance=strtod(str, NULL) * 100;
         if (! colon) printf("<H2>");

         if (balance < 0) printf("<font color=RED>Долг</font>: ");
         else printf("остаток: ");
         balance=abs(balance);
         printf("%d руб. %d коп.", balance/100, balance%100);
         if (! colon) printf("</H2>");
         break; 
      case ERR_NOACC:
         printf("<H2>нет счета</H2><br>(ваш хост не зарегистрирован)");
         exit(-1); 
      default:
         printf("<H2>Данные недоступны</H2><br>ошибка сервиса"); 
         exit(-1); 
   }

   link_puts(&lnk, "exit");
   link_close(&lnk);
   return 0;
}
