// File Shell
// Display file

#include "global.h"


AREA	Area[]=
{  {0,0,0,0,0,&Cmd,CmdProc},
   {0,0,0,0,0,0,PanelProc},
   {0,0,0,0,0,0,PanelProc},
   {0,0,0,0,0,0,KeyProc},
   {0,0,0,0,0,0,MenuProc},
   {0,0,0,0,0,0,0}    // terminator
};

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

