#include "global.h"

CONTROL loginDialogControls[]=
{  {  1,1,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Логин: \0",
      0,0,0,0,0
   },
   {  3,1,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Пароль: \0",
      0,0,0,0,0
   },
   {  1,9,1,16,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_EDIT,
      "\0",
      0,
      8,
      0,
      0,0
   },
   {  3,9,1,16,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_EDIT | CBT_PASSWORD,
      "\0",
      0,
      16,
      0,
      0,0
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

DIALOG  loginDialog=
{  0,0,
   5,26,
   DS_FRAMED,
   DA_DEFAULT,
   SI_NEUTRAL,
   "Аутентификация\0",
   0,0,
   0,
   0,0,
   loginDialogControls
};
