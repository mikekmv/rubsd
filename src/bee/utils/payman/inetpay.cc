#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <bee.h>
#include <ipc.h>

#include "global.h"
#include "inetpay.h"
#include "beetalk.h"
#include "userview.h"
#include "log.h"
#include "login.h"

#define PAY_MIN  (-10000)
#define PAY_MAX  50000

double   lastsum = 0;

int inetpayDialogProc(DIALOG * th, int action, uint param)
{
   if (action == PA_EPILOGUE)
   {  
      if (param == ID_OK)
      {  
         lastsum = strtod(((COMBOX*)(th->ctrl[1].mem))->buff, NULL);
      }
      return param;
   }

   if (action == PA_INITCTRL)
   {  ((COMBOX*)(th->ctrl[1].mem))->step = 50;
      return 0; 
   }

   return RET_CONT;
}

CONTROL inetpayDialogControls[]=
{  {  1,1,1,18,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"Сумма начисления: \0",
      0,0,0,0,0
   },
   {  1,20,1,12,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_COUNT | CBT_SIGNED | CBT_FLOAT,
      "\0",
      0,
      8,
      0,
      0,5
   },
   {  0,0,0,0,
      0,
      0,
      0,
      0,
      0,
      0,0,0,0,0
   }
};

DIALOG  inetpayDialog=
{  0,0,
   3,34,
   DS_FRAMED | DS_EPILOGED | DS_PROLOGED,
   DA_DEFAULT,
   SI_NEUTRAL,
   "Оплата Интернета\0",
   inetpayDialogProc,
   0,
   0,
   0,0,
   inetpayDialogControls
};

int InetPayment()
{  int    rc;
   char * msg;


// Trap N/A account
   if (UserView.user->inet_acc < 0)
   {  MessageBox("Операция невозможна\0",
           " Данная услуга не предоставляется \0",
           MB_OK | MB_NEUTRAL);
      return ID_CANCEL;
   }

// Trap Unlimited accounts
   if ((UserView.accstate1.tag & ATAG_UNLIMIT) != 0)
   {  MessageBox("Операция невозможна\0",
           " Операция запрещена на нелимитированом аккаунте \0",
           MB_OK | MB_NEUTRAL);
      return ID_CANCEL;
   }

// Get sum
   rc = inetpayDialog.Dialog(0);

// Trap Cancel
   if (rc != ID_OK) return rc;
   
// Trap out of range value
   if (lastsum > PAY_MAX || lastsum < PAY_MIN)
   {
      MessageBox("Недопустимый аргумент\0",
           " Неопустимая сумма (пределы -10000 - 50000)\0",
           MB_OK | MB_NEUTRAL);

      return ID_CANCEL;
   }

   RefreshConsole();
   uprintf("**** Начисляем %g на счет #%d ****\n\n", lastsum, UserView.user->inet_acc);
   uprintf("Соединение с билингом ... ");
   refresh();

   rc = bee_enter();
   if (rc < 0)
   {  uprintf("ОШИБКА.\n");
      uprintf("\n***** Нажмите любую клавишу *****\n"); 
      GetKey();
      return ID_CANCEL;
   }

   uprintf("OK\n");
   uprintf("Начисление ... ");
   refresh();
   rc = bee_send("add", "%d %g", UserView.user->inet_acc, lastsum);
   if (rc < 0)
   {  uprintf("ПРОГРАММНАЯ ОШИБКА.\n");
      uprintf("\n***** Нажмите любую клавишу *****\n"); 
      GetKey();
      bee_leave();
      return ID_CANCEL;
   }
    
   msg = NULL;
   rc = bee_recv(RET_SUCCESS, &msg, NULL);
   if (rc < 0 || rc > 400)
   {  uprintf("ОШИБКА (%d).\n", rc);
      uprintf("\n***** Нажмите любую клавишу *****\n"); 
      GetKey();
      bee_leave();
      return ID_CANCEL;
   }

   log_write("inetpay user \"%s\" acc %d sum %g by \"%s\"",
              UserView.user->regname,
              UserView.user->inet_acc,
              lastsum, 
              *loggeduser == '\0' ? "NOBODY" : loggeduser);

   uprintf("OK\n");
   uprintf("Отсоединяемся ... ");
   refresh();
   bee_leave();
   uprintf("OK\n");

//   GetKey();

   UserView.load_accs();

   return ID_OK;
}
