#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <bee.h>

#include "global.h"
#include "inetpay.h"
//#include "da.h"


int    lastsum = 0;

int inetpayDialogProc(DIALOG * th, int action, ulong param)
{
   if (action == PA_EPILOGUE)
   {  
      if (param == ID_OK)
      {  

         lastsum = strtol(((COMBOX*)(th->ctrl[1].mem))->buff, NULL, 10);
/*
         strlcpy(lastlogin,
              (char *)(((COMBOX*)(th->ctrl[2].mem))->buff),
              sizeof(lastlogin) );
         strlcpy(lastpass,
               (char *)(((COMBOX*)(th->ctrl[3].mem))->buff),
               sizeof(lastpass) );
*/
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
      CBT_COUNT | CBT_SIGNED | CBT_NUMERIC,
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
   "Начисление на Интернет\0",
   inetpayDialogProc,
   0,
   0,
   0,0,
   inetpayDialogControls
};

int InetPayment()
{  int rc;

// Get login/pass
   rc = inetpayDialog.Dialog(0);
// Trap Cancel
   if (rc != ID_OK) return rc;
   


   return ID_OK;
}
