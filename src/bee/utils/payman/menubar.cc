// Shell
// MenuBar procedure file

#include "global.h"

int PullDnProc       (DIALOG * th, int action, uint param);
int SubMenuProc	     (DIALOG * th, int action, uint param);

int	fInMenu;	// to prevent recursive call of PullDn dialog

#define MENUITEM(c,n,s) {0,0,0,0,CS_DEFAULT,(uint)(n),GenControl,CT_BUTTON|CT_MENULIKE,(c),0,0,0,0,(s)},
#define SEPARATOR {0,0,0,0,CS_DISABLED,0,GenControl,CT_SEPARATOR,"\0",0,0,0,0,0},
#define MENUEND  {  0,0,0,0,0,0,0,0,0,0,0,0,0,0  }

CONTROL LeftMenuControls[]=
{
   MENUITEM("Full\0",MI_LFULL,0)
   MENUITEM("Brief\0",MI_LBRIEF,1)
   MENUITEM("Custom ...\0",MI_LCUSTOM,0)
   MENUITEM("Info\0",MI_LINFO,0)
   MENUITEM("Tree\0",MI_LTREE,0)
   SEPARATOR
   MENUITEM("Sorting ...\0",MI_LSORT,0)
   MENUITEM("Filter ...\0",MI_LFILTER,0)
   SEPARATOR
   MENUITEM("Hide panel\0",MI_LHIDE,0)
   MENUITEM("Re-read panel\0",MI_LREREAD,0)
   SEPARATOR
   MENUITEM("Mount\0",MI_LMOUNT,0)
   SEPARATOR
   MENUITEM("Export panel\0",0,0)
   MENUEND
};

CONTROL FileMenuControls[]=
{
   MENUITEM("Choice No 1\0",MI_CH1,1)
   MENUITEM("Choice No 2\0",MI_CH2,0)
   MENUITEM("Choice No 3\0",MI_CH3,0)
   SEPARATOR
   MENUITEM("End NS Session.\0",K_F(10),0)
   MENUEND
};

CONTROL CommandsMenuControls[]=
{
   MENUITEM("Like command :)\0",0,0)
   {  0,0,0,0,0,0,0,0,0,0,0,0,0,0  }
};

CONTROL OptionsMenuControls[]=
{
   MENUITEM("Disable \"Mount\"\0",MI_MOUNTDISABLE,0)
   MENUITEM("Hide \"Mount\"\0",MI_MOUNTHIDE,0)
   MENUEND
};

CONTROL RightMenuControls[]=
{
   MENUITEM("Full\0",MI_RFULL,0)
   MENUITEM("Brief\0",MI_RBRIEF,1)
   MENUITEM("Custom ...\0",MI_RCUSTOM,0)
   MENUITEM("Info\0",MI_RINFO,0)
   MENUITEM("Tree\0",MI_RTREE,0)
   SEPARATOR
   MENUITEM("Sorting ...\0",MI_RSORT,0)
   MENUITEM("Filter ...\0",MI_RFILTER,0)
   SEPARATOR
   MENUITEM("Hide panel\0",MI_RHIDE,0)
   MENUITEM("Re-read panel\0",MI_RREREAD,0)
   SEPARATOR
   MENUITEM("Mount\0",MI_RMOUNT,0)
   SEPARATOR
   MENUITEM("Export panel\0",0,0)
   MENUEND
};

DIALOG LeftMenu=
{  1,2,
   1,20,
   DS_ARROWS|DS_PROLOGED|DS_PRIMARY|DS_FRAMED,
   DA_PROC,
   SI_MENUS,
   "Left\0",
   SubMenuProc,
   0,
   0,0,0,
   LeftMenuControls
};

DIALOG FileMenu=
{  1,2,
   0,0,
   DS_ARROWS|DS_PROLOGED|DS_PRIMARY|DS_FRAMED,
   DA_PROC,
   SI_MENUS,
   "File\0",
   SubMenuProc,
   0,
   0,0,0,
   FileMenuControls
};

DIALOG CommandsMenu=
{  1,2,
   1,20,
   DS_ARROWS|DS_PROLOGED|DS_PRIMARY|DS_FRAMED,
   DA_PROC,
   SI_MENUS,
   "Commands\0",
   SubMenuProc,
   0,
   0,0,0,
   CommandsMenuControls
};

DIALOG OptionsMenu=
{  1,2,
   1,20,
   DS_ARROWS|DS_PROLOGED|DS_PRIMARY|DS_FRAMED,
   DA_PROC,
   SI_MENUS,
   "Options\0",
   SubMenuProc,
   0,
   0,0,0,
   OptionsMenuControls
};

DIALOG RightMenu=
{  1,2,
   1,20,
   DS_ARROWS|DS_PROLOGED|DS_PRIMARY|DS_FRAMED,
   DA_PROC,
   SI_MENUS,
   "Right\0",
   SubMenuProc,
   0,
   0,0,0,
   RightMenuControls
};


CONTROL PullDnControls[]=
{
   {  0,1, 
      1,6,  
      CS_DEFAULT,
      (uint)&LeftMenu,
      GenControl,
      CT_BUTTON,
      " Left\0",
      0,0,0,0,0
   },
   {  0,12, 
      1,6,  
      CS_DEFAULT,
      (uint)&FileMenu,
      GenControl,
      CT_BUTTON,
      " File\0",
      0,0,0,0,0
   },
   {  0,23, 
      1,10,  
      CS_DEFAULT,
      (uint)&CommandsMenu,
      GenControl,
      CT_BUTTON,
      " Commands\0",
      0,0,0,0,0
   },
   {  0,37, 
      1,9,  
      CS_DEFAULT,
      (uint)&OptionsMenu,
      GenControl,
      CT_BUTTON,
      " Options\0",
      0,0,0,0,0
   },
   {  0,50, 
      1,7,  
      CS_DEFAULT,
      (uint)&RightMenu,
      GenControl,
      CT_BUTTON,
      " Right\0",
      0,0,0,0,0
   },
   {  0,0,0,0,0,0,0,0,0,0,0,0,0,0  }
};

DIALOG PullDn=
{  0,0,
   1,80,
   DS_ARROWS|DS_PROLOGED|DS_PRIMARY|DS_EPILOGED,
   DA_UP|DA_LEFT|DA_PROC,
   SI_PULLDN,
   0,
   PullDnProc,
   0,
   0,0,0,
   PullDnControls
};

int PullDnProc(DIALOG * th, int action, uint param)
{  static int fDrawOnly=0;
   
   switch (action)
   {  case PA_EPILOGUE:
         fInMenu=0;
	 return (int)param;
      case PA_PROLOGUE:
	 fInMenu=1;
      case PA_ALIGN:      
         th->w=ScreenColumns;
         {  int step,n,items=0;
	    CONTROL * ptr=th->ctrl;
            while (ptr->proc) if (!((ptr++)->style&CS_HIDDEN)) items++;
	    n=1;
            step=(ScreenColumns-5)/items;
	    for (ptr=th->ctrl;ptr->proc;ptr++)
	    {  if (!(ptr->style&CS_HIDDEN))
	       {  ptr->c=n;
	          n+=step;
	       }
            }
	 }
         if (action==PA_ALIGN) break;
         if (param==MP_DRAWONLY)
	 {  fDrawOnly=1;
            if ((keyLong&M_KEY)==K_TERMRESIZE) keyLong=0;
	    return RET_REDO;  
	 } else
	 {  fDrawOnly=0;
	 }
	 break;
      case PA_INITCTRL:
         break;
      case PA_PRIMEVENT:
         if (fDrawOnly)
	 {  CONTROL *ptr=th->ctrl+th->mem->Focus;
	    ptr->proc(ptr,th,CA_DRAW,0);
	    return RET_DEFEXIT;
	 }
	 switch (keyLong)
	 {  case K_ENTER:
	    case K_DNARROW:
	       {  int rets;
	          rets=((DIALOG*)((th->ctrl+th->mem->Focus)->id))->
	               Dialog((th->ctrl+th->mem->Focus)->c);
                  switch (rets)
		  {  case K_RTARROW:
		       {  CONTROL * ptr;
		          do
		          {  ptr=th->ctrl+(++(th->mem->Focus));
		             if (!(ptr->proc))
		             {  ptr=th->ctrl;
			        th->mem->Focus=0;
			     }
			  } while (ptr->style & CS_DISABLED);
              	       } 
		       th->mem->fDraw=1;
                       keyLong=K_ENTER;
                       return RET_REDO; 
		     case K_LTARROW:
		       {  CONTROL * ptr;
		          do
		          {  ptr=th->ctrl+(--(th->mem->Focus));
			     if (th->mem->Focus<0)
			     {  ptr=th->ctrl;
			        th->mem->Focus=(-1);
			        while (ptr++->proc) th->mem->Focus++;
				ptr--;
			     }
			  } while (ptr->style & CS_DISABLED);
              	       } 
		       th->mem->fDraw=1;
		       keyLong=K_ENTER;
		       return RET_REDO; 
		     default:
		       th->mem->Focus=rets;
	               return RET_DEFEXIT;
	          } // (switch)
	       } // (local area)
	       return RET_DONE;
	    case K_UPARROW:
	       return RET_DONE;
	 } // (switch key)  
	 break;
   } // (switch action)
   return RET_CONT;
}

int SubMenuProc(DIALOG * th, int action, uint param)
{  static int desiredCol;
     switch (action)
     {  case PA_PROLOGUE:
           desiredCol=(int)param;  
           {  CONTROL * ptr=th->ctrl;
	      int       lin=0;
	      int	cols, maxcols=0;
	      while (ptr->proc)
	      {  if (!(ptr->style & CS_HIDDEN))
	         {  ptr->l=lin;
		    lin++;
		    ptr->c=0;
		    ptr->h=1;
		    cols=ts(ptr->szzText,0,1)+3;
		    if (maxcols<cols) maxcols=cols;
		 }
                 ptr++;
	      }
	      th->l=1;
	      th->w=maxcols;
	      th->h=lin;
              ptr=th->ctrl;
              while (ptr->proc) (ptr++)->w=maxcols;
	   }
	   return RET_DONE;
        case PA_ALIGN:
           th->c=desiredCol;
           if (desiredCol<2) th->c=2;
	   else if (desiredCol+th->w+4>ScreenColumns) 
	        {  th->c=ScreenColumns - th->w - 4;
		}
	   return 0;
	case PA_PRIMEVENT:
	   switch (keyLong)
	   {  case K_LTARROW:
	      case K_RTARROW:
	         th->mem->Focus=(int)keyLong;
		 return RET_DEFEXIT;
	   } break;
   }
   return RET_CONT;
}
int	MenuProc (AREA * th, int action, uint param)
{   switch (action)
    {   case OA_DISPEVENT:
           return RET_CONT;
	case OA_MINORUPD:
	case OA_MAJORUPD:

	   if (! fInMenu)
           {  int min=fMinorUpdate;
	      int maj=fMajorUpdate;
	      fMajorUpdate=0;
	      fMinorUpdate=0;
              PullDn.Dialog(MP_DRAWONLY);
	      fMajorUpdate=maj;
	      fMinorUpdate=min;
	   }
	   
	   return 0;
	default:
	   return RET_CONT;
    }
}
