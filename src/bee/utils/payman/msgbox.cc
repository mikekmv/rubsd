// File shell
// Simple message box file
//
// functions:
//
//

#include "global.h"

int MsgBoxProc(DIALOG * th, int action, uint param);

class MBBUTTONS
{  public:
   char  * text[3];  // (button texts)
   uint    id[3];    // (button ids)
   int     style[3]; // (status - for hidden/visible)
   int     disp[3];  // (col disp (central column relative))
};

class MBPARAM
{  public:
   char	* title;
   char * text;
   int    type;
   int    defb;
   int	  scheme;
};

char 	szzOK[]    ="   OK\0";
char	szzCancel[]=" Отмена\0";
char	szzYes[]   ="   Да\0";
char	szzNo[]    ="  Нет\0";

MBBUTTONS     MBbuttons[]=
{  {  {szzOK,0,0},                   // MB_OK
      {ID_OK,0,0},
      {  0,
         CS_HIDDEN|CS_DISABLED,
	 CS_HIDDEN|CS_DISABLED},
      {(-4),0,0}
   },
   {  {szzOK,szzCancel,0},           // MB_OKCANCEL
      {ID_OK,ID_CANCEL,0},
      {  0,
         0,
	 CS_HIDDEN|CS_DISABLED},
      {(-9),1,0}
   },
   {  {szzYes,szzNo,0},              // MB_YESNO
      {ID_YES,ID_NO,0},
      {  0,
         0,
	 CS_HIDDEN|CS_DISABLED},
      {(-9),1,0}
   },
   {  {szzYes,szzNo,szzCancel},           // MB_YESNOCANCEL
      {ID_YES,ID_NO,ID_CANCEL},
      {0,0,0},
      {(-14),(-4),6}
   }
};

CONTROL MsgBoxControls[]=
{  {  0,0,0,0,			// dinamic counted
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
      0,
      0,0,0,0,0
   },
   {  0,0,1,8,
      0,
      0,
      GenControl,
      CT_BUTTON,
      0,
      0,0,0,0,0
   },
   {  0,0,1,8,
      0,
      0,
      GenControl,
      CT_BUTTON,
      0,
      0,0,0,0,0
   },
   {  0,0,1,8,
      0,
      0,
      GenControl,
      CT_BUTTON,
      0,
      0,0,0,0,0
   },
   {  0,0,0,0,		// terminator
      0,
      0,
      0,
      0,
      0,
      0,0,0,0,0
   }

};

DIALOG MsgBoxDialog=
{  0,0,
   17,46,
   DS_FRAMED|DS_PROLOGED|DS_ARROWS,
   DA_PROC,				//for focus setting
   SI_NEUTRAL,
   0,
   MsgBoxProc,
   0,
   0,
   0,0,
   MsgBoxControls
};

int MessageBox(char * title, char * text, int type)
{  MBPARAM param=
   {  title,text,
      type&MBM_TYPE,
      (type&MBM_DEF)>>MBS_DEF,
      (type&MBM_SCHEME)>>MBS_SCHEME
   };
   return MsgBoxDialog.Dialog((uint)&param);
}

int MsgBoxProc(DIALOG * th, int action, uint param)
{  static int defb=0;
   static int fSetFocus=0;
#define param (*((MBPARAM*)param))
   switch(action)
   {  case PA_PROLOGUE:
         fSetFocus=1;
         th->title=param.title;
	 th->scheme=param.scheme;
         defb=param.defb;
	 {  CONTROL   * stat=th->ctrl;
	    MBBUTTONS * bptr=MBbuttons+param.type;
            int 	ccol,i=0;
	    stat->szzText=param.text;
	    ts(param.text,stat,1);
	    th->w=stat->w;
	    th->h=stat->h+2;
//Gotoxy(0,0);printf("hight=%d(%d)",stat->h,th->ctrl->h);getch(); 
            ccol=stat->w/2+1;
	    for (CONTROL * ptr=th->ctrl+1;ptr->proc;ptr++)
	    {  ptr->szzText=bptr->text[i];
	       ptr->id=bptr->id[i];
	       ptr->style=bptr->style[i];
	       ptr->l=stat->h+1;
	       ptr->c=ccol+bptr->disp[i];
               i++;
	    }
	 }
	 break;
      case PA_ALIGN:
         if (fSetFocus) th->mem->Focus=defb+1;
	 fSetFocus=0;
	 return 1;
   }
   return RET_CONT;
}
