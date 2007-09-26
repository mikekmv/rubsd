
#include "userview.h"
#include "global.h"
#include "list.h"


USERVIEW  UserView;


int  USERVIEW::load_accs()
{  accbase_t  based;
   int        rc;

   if (user == NULL) return 0;

   accflag1 = accflag2 = (-32000);

   if (user->inet_acc < 0)  accflag1 = NOT_FOUND;
   if (user->intra_acc < 0) accflag2 = NOT_FOUND;

   rc = acc_baseopen(&based, conf_accfile);
   if (rc < 0) return (-1);

   accflag1 = accflag2 = NOT_FOUND;

   if (user->inet_acc >= 0)
      accflag1 = acc_get(&based, user->inet_acc, &accstate1);

   if (user->intra_acc >= 0)
      accflag2 = acc_get(&based, user->intra_acc, &accstate2);
      
   acc_baseclose(&based);

   return 0;      
}

int  USERVIEW::refresh()
{  int     i;
   int     tempval;
   
   time_t     tim;
   struct tm  stm;

   if ((flags & UVF_REFRESH) != 0)
   {  Gotoxy(2, 2); Attr(7, 0);
      uprintf("  Пользователь: ", user->regname);
      Attr(15, 0);
      uprintf("%s", user->regname);

      Gotoxy(3, 2); Attr(7, 0);
      uprintf("         Счета: ");
      Attr(15, 0);
      if (user->inet_acc >= 0)
         uprintf("# %d (инет) ", user->inet_acc);
      if (user->intra_acc >= 0)
         uprintf("# %d (сеть)", user->intra_acc);

      Gotoxy(4, 2); Attr(7, 0);
      uprintf("         Хосты: ");
      Attr(15, 0);
      if (user->cnt_hosts < 1)
         uprintf("-");
      else
         for (i=0; i < user->cnt_hosts; i++)
         {  uprintf("%s/%d",
            inet_ntoa( *((in_addr*)&(user->itm_hosts[i].addr)) ),
            user->itm_hosts[i].mask );
            if (i < (user->cnt_hosts - 1) )
               uprintf(", ");
         }

      Gotoxy(6, 2); Attr(7, 0);
      uprintf("Почтовые ящики: ");
      Attr(15, 0);
      if (user->cnt_mail < 1)
         uprintf("-");
      else
         for (i=0; i < user->cnt_mail; i++)
         {  uprintf("%s@%s",
                user->itm_mail[i].login,
                user->itm_mail[i].domain );
            if (i < (user->cnt_mail - 1) )
            uprintf(", ");
         }

      Gotoxy(7, 2); Attr(7, 0);
      uprintf("         Порты: ");
      Attr(15, 0);
      if (user->cnt_ports < 1)
         uprintf("-");
      else
         for (i=0; i < user->cnt_ports; i++)
         {  uprintf("%s:%d",
            user->itm_ports[i].switch_id,
            user->itm_ports[i].port );
            if (i < (user->cnt_ports - 1) )
               uprintf(", ");
         }

      if (accstate1.tariff != 0)
      {  Gotoxy(8, 2); Attr(7, 0);
         uprintf("         Тариф: ");
         Attr(15, 0);
         uprintf("%d ", accstate1.tariff);
         switch(accstate1.tariff)
         {  case 3:
               uprintf("(безлимит-64)");
               break;
            case 4:
               uprintf("(безлимит-128)");
               break;
         }
      }

      Gotoxy(11, 12); Attr(7, 0);
      uprintf("Текущее состояние:");

// Dynamic data
      Gotoxy(13, 6); Attr(7, 0);
      uprintf("Инет: ");
      Attr(15, 0);
      switch(accflag1)
      {  case (-32000):
         case IO_ERROR:
            uprintf("Данные недоступны ");
            if (accflag1 == (-32000)) uprintf("(нет доступа к базе)");
            if (accflag1 == IO_ERROR) uprintf("(ошибка ввода/вывода)");
            break;
         case NOT_FOUND:
         case ACC_DELETED:
            Attr(7, 0);
            uprintf("услуга не предоставляется ");
            if (accflag1 == ACC_DELETED) uprintf("(счет удален)");
            break;
         case ACC_BROKEN:
            uprintf("СЧЕТ ПОВРЕЖДЕН, сообщите администратору !");
            break;            

         default:
            if (accflag1 == ACC_FROZEN) uprintf("Счет заморожен ");
            if ((accstate1.tag & ATAG_UNLIMIT) != 0) uprintf("UNLIMIT ");

            tempval = (int)(accstate1.balance * 100);
             
            if ((accstate1.tag & ATAG_UNLIMIT) == 0 || tempval != 0)
            {  if (tempval < 0) uprintf("долг ");
               else uprintf("остаток ");

               tempval = abs(tempval);
            
               uprintf("%d.%02d ", tempval/100, tempval%100);
            }  

            if (accstate1.start != 0 || accstate1.stop != 0)
            {  Attr(7, 0);
               uprintf("(косяки в настройке счета)");
            }
      }

      Gotoxy(15, 6); Attr(7, 0);
      uprintf("Сеть: ");
      Attr(15, 0);
      switch(accflag2)
      {  case (-32000):
         case IO_ERROR:
            uprintf("Данные недоступны ");
            if (accflag2 == (-32000)) uprintf("(нет доступа к базе)");
            if (accflag2 == IO_ERROR) uprintf("(ошибка ввода/вывода)");
            break;
         case NOT_FOUND:
         case ACC_DELETED:
            Attr(7, 0);
            uprintf("услуга не предоставляется ");
            if (accflag2 == ACC_DELETED) uprintf("(счет удален)");
            break;
         case ACC_BROKEN:
            uprintf("СЧЕТ ПОВРЕЖДЕН, сообщите администратору !");
            break;            

         default:
            if (accflag2 == ACC_FROZEN)  uprintf("Счет заморожен ");
            if ((accstate2.tag & ATAG_UNLIMIT) != 0) uprintf("UNLIMIT ");
            else            
            {  
               if (accstate2.start != 0 || accstate2.stop != 0)
                  uprintf("оплачено ");
 
               tim = time(NULL);
   
               if ((accstate2.start > tim) ||
                   (accstate2.start > 0 && accstate2.stop == 0))
               {  if (localtime_r(&(accstate2.start), &stm) != NULL)
                     uprintf("с %s '%02d ", months_cased[stm.tm_mon], stm.tm_year%100);
               }
 
               if (accstate2.stop)
               {  if (localtime_r(&(accstate2.stop), &stm) != NULL)
                     uprintf("по %s '%02d ", 
                         months[ stm.tm_mon ? (stm.tm_mon - 1) : 11 ],
                         stm.tm_year%100);
               }

               if ((accstate2.start < tim && accstate2.stop > tim) ||
                   (accstate2.start > 0 && accstate2.stop == 0))
               {  if (user->cnt_ports == 0)
                     uprintf("(включен ВРУЧНУЮ) ");
               }  
               else
               {  if (user->cnt_ports != 0) uprintf("(отключен)");
                  else uprintf("(отключен ВРУЧНУЮ) ");
               }  
            } 

            if (accstate2.balance >= 0.01)
            {  Attr(7, 0);
               uprintf("(косяки в настройке счета)");
            }

      } //switch

   } // if refresh

   return 0;
}

