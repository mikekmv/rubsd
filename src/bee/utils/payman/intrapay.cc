#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <bee.h>
#include <ipc.h>

#include "global.h"
#include "intrapay.h"
#include "beetalk.h"
#include "userview.h"


//#include "da.h"


int    lmonths    = 0;
int    lmonthfrom = 0;

int intrapayDialogProc(DIALOG * th, int action, ulong param)
{
   if (action == PA_EPILOGUE)
   {  
      if (param == ID_OK)
      {  
//         lastsum = strtol(((COMBOX*)(th->ctrl[1].mem))->buff, NULL, 10);
      }
      return param;
   }

   if (action == PA_INITCTRL)
   {  ((COMBOX*)(th->ctrl[1].mem))->step = 50;
      return 0; 
   }

   return RET_CONT;
}

CONTROL intrapayDialogControls[]=
{  
   {  1,1,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"Оплачено\0",
      0,0,0,0,0
   },
   {  1,10,1,4,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_COUNT | CBT_NUMERIC,
      "\0",
      0,
      2,
      0,
      0,5
   },
   {  1,15,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"месяцев\0",
      0,0,0,0,0
   },
   {  3,1,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
//2345678901234567890
"начиная с\0",
      0,0,0,0,0
   },
   {  3,10,1,10,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_DROPDN,
      "\0",
      0,
      2,
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

DIALOG  intrapayDialog=
{  0,0,
   5,34,
   DS_FRAMED | DS_EPILOGED | DS_PROLOGED,
   DA_DEFAULT,
   SI_NEUTRAL,
   "Начисление на сеть\0",
   intrapayDialogProc,
   0,
   0,
   0,0,
   intrapayDialogControls
};

int IntraPayment()
{  int    rc;
   char * msg;


// Trap N/A account
   if (UserView.user->intra_acc < 0)
   {  MessageBox("Операция невозможна\0",
           " Данная услуга не предоставляется \0",
           MB_OK | MB_NEUTRAL);
      return ID_CANCEL;
   }

// Get sum
   rc = intrapayDialog.Dialog(0);

// Trap Cancel
   if (rc != ID_OK) return rc;
   
// Trap out of range value
   if (lastsum > 10000 || lastsum < -5000)
   {  MessageBox("Недопустимый аргумент\0",
           " Недопустимая сумма начисления \0",
           MB_OK | MB_NEUTRAL);

      return ID_CANCEL;
   }

/*

   RefreshConsole();
   uprintf("**** Начисляем %d на счет #%d ****\n\n", lastsum, UserView.user->intra_acc);
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
   rc = bee_send("add", "%d %d", UserView.user->inet_acc, lastsum);
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

   uprintf("OK\n");
   uprintf("Отсоединяемся ... ");
   refresh();
   bee_leave();
   uprintf("OK\n");

//   GetKey();
*/

   UserView.load_accs();

   return ID_OK;
}
