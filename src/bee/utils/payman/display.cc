// File Shell
// Display file

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "global.h"
#include "list.h"
#include "userview.h"

AREA	Area[]=
{  {0,0,0,0,0,&Cmd,CmdProc},
   {0,0,0,0,0,0,PanelProc},
   {0,0,0,0,0,0,PanelProc},
   {0,0,0,0,0,0,KeyProc},
   {0,0,0,0,0,0,MenuProc},
   {0,0,0,0,0,0,0}    // terminator
};


int StageScr = 0;
int DoRefresh = 0;

int	Update()
{
//   int i;

   if (DoRefresh != 0)
   {  Attr(7, 0);
      clear();
      RefreshTitle();
      RefreshKeybar();
      Attr(7, 0);
      Gotoxy(2,0);
      

      UserList.flags = ULF_REFRESH;
      UserView.flags = UVF_REFRESH; 
   }   

   switch (StageScr)
   {  case 0:
         UserList.refresh();
         break;
      case 1:
         UserView.refresh();
         break;
      case 2:
      case 3:
         RefreshHelp();
         break;
   }

// Show notice
   if (StageScr < 2)
   {  Gotoxy(ForceLins - 3, 4);
      Attr(7, 0);
      uprintf("ПОМНИТЕ: Состояние счета пользователя - ");
      uprintf("конфиденциальная информация");
   }
// Park cursor
   Gotoxy(ForceLins - 3, 1);

   DoRefresh = 0;

   return 0;
}

#ifdef NEVER
int	Update()
{   // debug - clear screen if MajorUpdate
//    if (fMajorUpdate) erase();
    if (fMajorUpdate || fMinorUpdate)
    {   AREA  *  ptr=Area;
        /* int  n=0; //unused variable */ 
        do
        {   if (!(ptr->lins)) continue;
	    if (fMajorUpdate)
            {  (ptr->proc)(ptr, OA_MAJORUPD,0);
	       refresh();
	    }
            else if (fMinorUpdate)
            {  (ptr->proc)(ptr, OA_MINORUPD,0);
	       refresh();
	    }
	} while ((ptr++)->proc);
	fMajorUpdate=0;
        fMinorUpdate=0;
    }
    {  register AREA * ptr=&Area[AI_CMDVIEW];
       ptr->proc(ptr,OA_PUTCURSOR,0);
       Cursor(1);
    }
    return 0;
}
#endif
