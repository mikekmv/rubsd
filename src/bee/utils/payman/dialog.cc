// File shell
// Dialogs class file
//
// functions:
//
// DIALOG::Dialog   - Do dialog Default job
// DIALOG::Dispatch - (Dialog) Do dispatch job
// DIALOG::SetFocus - (Dialog) Switch keyboard Focus & notify
// 
// GenControl       - Simple controls handling procedure (static, button,
//                    checkbox, groupbox)
// RadioBoxControl  - Radiobutton handling procedure
// ListBoxControl   - Listbox handling procedure
// ComboBoxControl  - Edit/combo-box handling procedure
//

#include "global.h"

char   * testList[]=
{ "(none)",
  "Black",
  "Blue",
  "Red",
  "Magenta",
  "Green",
  "Cyan",
  "Yellow",
  "White",
  "(inverse)"
};

LISTBOX	testListBox=
{  10,0,testList
};

char Array[32]="";
char Array2[32]="";
char Array3[32]="";
char Array4[32]="";

COMBOX ComboMem=
{  0,
   0,
   0,
   Array,
   1,
   6,
   18,
   { 10,0,testList }  
};

COMBOX ComboMem2=
{  0,
   0,
   0,
   Array2,
   1,
   6,
   18,
   { 10,0,testList }  
};

COMBOX ComboMem3=
{  0,
   0,
   0,
   Array3,
   1,
   6,
   18,
   { 10,0,testList }  
};

COMBOX ComboMem4=
{  0,
   0,
   0,
   Array4,
   1,
   6,
   18,
   { 10,0,testList }  
};

CONTROL testDialogControls[]=
{  {  0,0,2,46,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"   It's Static text control, that"
" prompting user what to do ... :)\0",
      0,0,0,0,0
   },
   {  3,1,1,16,
      CS_DEFAULT,
      0xAA55,
      GenControl,
      CT_CHECKBOX,
"CheckBox #1\0",
      0,0,0,0,0
   },
   {  4,1,1,16,
      CS_DEFAULT,
      0xAA55,
      GenControl,
      CT_CHECKBOX,
"CheckBox #2\0",
      0,0,0,0,0
   },
   {  5,1,6,20,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_GROUP,
      "GroupBox\0",
      0,0,0,0,0
   },
   {  6,3,1,16,
      CS_DEFAULT,
      0xAA55,
      RadioBoxControl,
      0,
"RadioBox #1\0",
      0,0,0,0,0
   },
   {  7,3,1,16,
      CS_DEFAULT,
      0xAA55,
      RadioBoxControl,
      0,
"RadioBox #2\0",
      0,0,0,0,0
   },
   {  8,3,1,16,
      CS_DEFAULT,
      0xAA55,
      RadioBoxControl,
      0,
"RadioBox #3\0",
      0,0,0,0,0
   },
   {  9,3,1,16,
      CS_DEFAULT,
      0xAA55,
      RadioBoxControl,
      0,
"RadioBox #4\0",
      0,0,0,0,0
   },
   {  5,23,6,20,
      CS_EXTERN,
      0xAA55,
      ListBoxControl,
      0,
      "ListBox\0",
      0,0,
      &testListBox,0,0
   },
   {  11,3,1,16,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Dropdown:\0",
      0,0,0,0,0
   },
   {  12,3,1,16,
      CS_EXTERN,
      0xAA55,
      ComboBoxControl,
      CBT_DROPDN,
      "\0",
      0,
      30,
      &ComboMem,
      0,0
   },
   {  11,24,1,16,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Simple list:\0",
      0,0,0,0,0
   },
   {  12,24,1,16,
      CS_EXTERN,
      0xAA55,
      ComboBoxControl,
      CBT_SIMPLE | CBT_READONLY,
      "\0",
      0,
      30,
      &ComboMem2,
      0,0
   },
   {  13,3,1,16,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Count combo:\0",
      0,0,0,0,0
   },
   {  14,3,1,16,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_COUNT|CBT_FLOAT|CBT_SIGNED,
      "\0",
      0,
      10,
      0,
      0,0
   },
   {  13,24,1,16,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Edit only:\0",
      0,0,0,0,0
   },
   {  14,24,1,16,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_EDIT,
      "\0",
      0,
      30,
      0,
      0,0
   },
   {  2,34,1,8,
      CS_DEFAULT,
      0xAA55,
      GenControl,
      CT_BUTTON,
      "   OK\0",
      0,0,0,0,1
   },
   {  4,34,1,8,
      CS_DEFAULT,
      0xAA55,
      GenControl,
      CT_BUTTON,
      " Cancel\0",
      0,0,0,0,0
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
DIALOG	testDialog=
{  0,0,
   17,46,
   DS_FRAMED,
   DA_DEFAULT,
   SI_NEUTRAL,
   "Demo dialog\0",
   0,0,
   0,
   0,0,
   testDialogControls
};

int	DIALOG::Dialog(ulong param)
{   int 	rets=RET_CONT;
    int 	fDrawFocus = 0;
    LISTBOX	list;
// Initialize controls before allocation (set default sizes)
    {  CONTROL * ptr=ctrl;
       // int       first=1;
       while (ptr->proc)
       {  if (!(ptr->style & CS_EXTERN || bytes || ptr->proc==GenControl))
          {  if (ptr->proc==ListBoxControl)
	     {  ptr->bytes=sizeof(LISTBOX);
	        if (!(ptr->style & CS_EXTLST))
		   ptr->bytes+=ptr->limit*sizeof(void*);
	     }
             if (ptr->proc==ComboBoxControl)
	     {	ptr->bytes=sizeof(COMBOX);
	        if (!(ptr->style & CS_EXTBUF))
		   ptr->bytes+=ptr->limit+1;
		if (!(ptr->style & CS_EXTLST))   
		   ptr->bytes+=((ptr->type&CBT_MASK)<CBT_SIMPLE ? 
		               0:((int)ptr->szzText)*sizeof(void*));
	     }
	  }             
          ptr++;
       } // while
    }
// Prologue
    if ((style & DS_PROLOGED) && proc) rets=proc(this,PA_PROLOGUE,param);
// Allocating memory
    if (! (style & DS_STATIC))
    {  bytes=sizeof(DIALOGDATA);
       bytes+=procbytes;
       for (CONTROL * ptr=ctrl; ptr->proc; ptr++)
          if (! (ptr->style & CS_EXTERN) )
              bytes+=ptr->bytes;
       mem=(DIALOGDATA *)new char[bytes];
       if (! mem) return (ID_MEMOUT);
    }
// Set up controls memory ptrs
    {  char * ptr=(char *) mem;
       ptr+=sizeof(DIALOGDATA);
       procmem=(void *)ptr;
       ptr+=procbytes;
       for (CONTROL * cptr=ctrl; cptr->proc; cptr++)
           if (! (cptr->style & CS_EXTERN) && cptr->bytes  )
	   {  cptr->mem=(void *)ptr;
	      ptr+=cptr->bytes;
	   }
    }
// Initialize controls (dinamic data)
    {  CONTROL * ptr=ctrl;
       int       first=1;
       while (ptr->proc)
       {  if (ptr->proc==ListBoxControl) 
	  {  if (!(ptr->style & CS_EXTERN))
	     {  ((LISTBOX*)(ptr->mem))->pcs=0;
	        if (ptr->style & CS_EXTLST) ((LISTBOX*)(ptr->mem))->array=0;
	        else
		   ((LISTBOX*)(ptr->mem))->array=
		     (char**)(((char*)ptr->mem)+sizeof(LISTBOX));
	        
	     }
	     ptr->val=0;
	     ((LISTBOX*)(ptr->mem))->disp=0;
	  }
          if (ptr->proc==RadioBoxControl)
	  {  if (first)
	     {  ptr->val=1;
	        first=0;
	     } else ptr->val=0;
	  } else first=1;
          ptr++;

	  if (ptr->proc==ComboBoxControl)
	  {  if (!(ptr->style & CS_EXTERN))
	     {  if (ptr->style & CS_EXTBUF) ((COMBOX*)(ptr->mem))->buff=0;
	        else
		   ((COMBOX*)(ptr->mem))->buff=
	            (((char*)ptr->mem)+sizeof(COMBOX));
		if (ptr->style & CS_EXTLST) ((COMBOX*)(ptr->mem))->lst.array=0;
		else
		{  ((COMBOX*)(ptr->mem))->lst.array=(char**)(((char*)ptr->mem)+
		    (ptr->style & CS_EXTBUF?0:(sizeof(COMBOX)+ptr->limit+1)));
	        } 	    
		    ((COMBOX*)(ptr->mem))->lst.pcs=0;
	     }
	     switch (ptr->type&CBT_MASK)
	     {  case CBT_EDIT:
	           ptr->proc(ptr,this,CA_SETTEXT|CAS_NODRAW,(ulong)"");
		   break;
		case CBT_COUNT:
		   ptr->proc(ptr,this,CA_SETTEXT|CAS_NODRAW,(ulong)"");
		   break;
		default:
		   if ( ((COMBOX*)(ptr->mem))->lst.pcs )
		      ptr->proc(ptr,this,CA_SET|CAS_NODRAW,0);
		   else
	           ptr->proc(ptr,this,CA_SETTEXT|CAS_NODRAW,(ulong)"");
	     } 
	     if (ptr->type & CBT_FLOAT) *((float*)(&ptr->val))=0;
	     else ptr->val=0;
	     ((COMBOX*)(ptr->mem))->lst.disp=0;
	     ((COMBOX*)(ptr->mem))->step=ptr->type&CBT_FLOAT?0.1:1;
//	     ((COMBOX*)(ptr->mem))->h=4;
//	     ((COMBOX*)(ptr->mem))->w=ptr->w;
          }
          ptr->notify=0;
       }
       mem->FLctrl=ptr;
    }
    mem->fFList=0;
    mem->FList=&list;
// Prologue for controls dinamic data
    if ((style & DS_PROLOGED) && proc) proc(this,PA_INITCTRL,0);
// Set Keyboard Focus (to first enabled control)
    {  CONTROL * ptr=ctrl;
       mem->Focus=0;
       while ((ptr++)->style & CS_DISABLED)
       {  if (! ptr->proc)
          {  mem->Focus=0;
	     break;
	  }
          mem->Focus++;
       }
       ptr=ctrl+mem->Focus;
       if (!(ptr->style & CS_DISABLED) && (style & DS_NOTIFY) && proc)
       {  ptr->notify=CN_FOCUS;
          proc(this, PA_NOTIFY, mem->Focus);
       }
    }
// External cycle
    mem->fDraw=1;
    do
    {  
// (Re-)align dialog    
       int flag=1;
       if ((align & DA_PROC) && proc)
       {  flag=proc(this,PA_ALIGN,0);
       } 

       if (flag)
       {  if (align & DA_NOCENTERL)
          {  if (align & DA_LINE)		// Down
	        l=ScreenLines-h;
 	     else      				// Up
	        l=0;
          } else
	  {  if (align & DA_HALFL)				
	     {  if (align & DA_LINE)		// Downhalf 
	        {  float half=((float)ScreenLines)/2;
		   l=( (((int)half)-h)/2+((int)(half+0.5)) );

		   if (l+h>ScreenLines) l=ScreenLines-h;
		} else				// Uphalf
		{  l=(((int)ScreenLines/2)-h)/2;
		   if (l<0) l=0;
		}
	     } else				// Center
	        l=(ScreenLines-h)/2;
	  }  
          if (align & DA_NOCENTERC)
          {  if (align & DA_COLUMN)		// Right
	        c=ScreenColumns-w;		
	     else				// Left
	        c=0;				
          } else
	  {  if (align & DA_HALFC)				
	     {  if (align & DA_COLUMN)		// Righthalf 
	        {  float half=((float)(ScreenColumns))/2;
		   c=( (((int)half)-w)/2+((int)(half+0.5)) );
		   if (c+w>ScreenColumns) c=ScreenColumns-w;
		} else				// Lefthalf
		{  c=(((int)ScreenColumns/2)-w)/2;
		   if (c<0) c=0;
		}
	     } else				// Center
 	        c=(ScreenColumns-w)/2;
	  }
       }	  
       if (rets==RET_TERM) Arrange();
// Internal cycle
       do
       {
// Update
	 if (rets==RET_REDRAW)
	    fMinorUpdate=1;  // to redraw screen on dialog redrawing
// 1. Downlaid screen (If Needed)
         if (fMinorUpdate || fMajorUpdate)
	 {  ::Update();				
	    mem->fDraw=1;
	 }
// 2. Dialog (IN)
	  if (mem->fDraw)
	  {  
	      WINOUT   Cwin;
              char    *p;
	      int      i,il;
	      Cwin.New(this);
              bAttr(Schemes[scheme]->frame);
              if (style&DS_FRAMED)
	      {  p=Chars;
	         Cwin.lin--;
                 Cwin.lins+=3;
	         Cwin.col-=2;
	         Cwin.cols+=6;
                 Cwin.at(0,0);
	         Cwin.pc(' ');
	         Cwin.pc(*(p++));               // #
	         if (title && w>4)
	         {  int n=strlen(title);
                    Cwin.pc(*p);                // =
	            if (w-n>3)
		    {  i=(int)((w-n-4)/2);
		       while(i--) Cwin.pc(*p);  // =
		       Cwin.pc(' ');            // 
		       Cwin.ps(title);
		       Cwin.pc(' ');            //
		       i=(int)((float)(w-n-4)/2+0.6);
		       while(i--) Cwin.pc(*p);  // = 
		    } else
		    {  Cwin.pc(' ');            // 
		       if (w-4>3)
		       {  Cwin.psfd(title, w-7);
		          Cwin.ps("...");
		       } else
		       {  Cwin.psfd(title, w-4);
		       }
		       Cwin.pc(' ');            // 
		    }
		    Cwin.pc(*p++);              // =
	         } else
	         {  i=w;
		    while(i--) Cwin.pc(*p);     // =
		    p++;
	         }
                 Cwin.pc(*(p++));               // #
	         Cwin.pc(' ');		     //	
	         Cwin.pc('\n');
	      } else p=" ";
 	      il=h;
	      while (il--)
	      {  if (style&DS_FRAMED)
	         {  Cwin.pc(' ');		    //
	            Cwin.pc(*(p++));	    // |
		 }
		 i=w;
		 while (i--) Cwin.pc(*p);   // 
		 if (style&DS_FRAMED)
		 {  Cwin.pc(*(++p));           // |
		    Cwin.pszz(" \0c\7\0  \0");   // +shadow
	            bAttr(Schemes[scheme]->frame);
		    p-=2;
		 }
	      }
	      if (style&DS_FRAMED)
	      {  p+=3;
	         Cwin.pc(' ');
	         Cwin.pc(*(p++));
                 i=w;
                 while (i--) Cwin.pc(*p);
                 Cwin.pc(*(++p));
	         Cwin.pszz(" \0c\7\0  \0");
                 Cwin.wcol+=2;
	         Cwin.at(Cwin.wlin,Cwin.wcol);
	         i=w+4;
	         while (i--) Cwin.pc(' ');
              }  
              Defs.New(this);
	      {  CONTROL * ptr=ctrl;
	         do
		 {  if (!(ptr->style&CS_HIDDEN))
		       ptr->proc(ptr, this, CA_DRAW,0);
		    ptr++;
		 } while (ptr->proc);
	      }
	      mem->fDraw=0;
              if (mem->fFList) mem->fDrawCur=1;
	      else mem->fDrawCur=0;
	      fDrawFocus=1;
	  }
// 3. Current control (IN)
          if (mem->fDrawCur)
	  {  register CONTROL * ptr;
	     if (mem->fFList)
	     {  
	        ListBoxControl(mem->FLctrl,this,CA_DRAW,0);
	     } else
	     {  ptr=ctrl+mem->Focus;
	        ptr->proc(ptr,this,CA_DRAW,0);
	     }
             mem->fDrawCur=0;
	     fDrawFocus=1;
	  }     
// 4. Focus (IN) 
	  if (fDrawFocus)
	  {  if (mem->fFList)
	     {  ListBoxControl(mem->FLctrl,this,CA_DRAWFOCUS,0);
	     } else
	     {  register CONTROL * ptr=ctrl+mem->Focus;
	        ptr->proc(ptr,this,CA_DRAWFOCUS,0);
	     }
	     fDrawFocus=0;
	  }
          //--------------------------------------------
// Waiting "event"
	  if (rets!=RET_REDO)
	     WaitEvent();
	  //--------------------------------------------
// Dispatching
          rets=Dispatch();
// Internal cycle end
       } while (rets==RET_DONE || rets==RET_REDO);
// External cycle end
    } while (rets==RET_TERM || rets==RET_REDRAW);
// Kill Focus Notify
    if (rets!=RET_DEFEXIT && (style & DS_NOTIFY) && proc) 
    {  CONTROL * fctrl=ctrl+mem->Focus;
       fctrl->notify=CN_KILLFOCUS;
       proc(this, PA_NOTIFY, mem->Focus);
    }
// Get return value
    if (rets==RET_DEFEXIT)
       rets=mem->Focus;
    else   
       rets=(ctrl+mem->Focus)->id;
// Epilogue
    if ((style & DS_EPILOGED) && proc) rets=proc(this,PA_EPILOGUE,rets);
// Free memory
    if (! (style & DS_STATIC)) delete [] mem;
// Set screen update
    fMinorUpdate=1;
// Guess what ...
    return rets;
}

//        Queue
// 1. Program keys (TermResize etc.)
// 2. Primary proc dispatch
// 3. Focus Control
// 4. Dialog
// 5. Proc dispatch
// 6. (Error beep)

int	DIALOG::Dispatch()
{   CONTROL   * fctrl=ctrl+mem->Focus;
    int	 	rets=RET_CONT;
// ---------------------------- Program ----------------------------
    if (keyLong==K_TERMRESIZE)
    {  GetSizes();
       return RET_TERM;
    }
// --------------- Fictive ListBox (combo droplist) ----------------
    if (mem->fFList)
    {   return ListBoxControl(mem->FLctrl,this,CA_EVENT,0);
    }
// ---------------------------- Primary ----------------------------
    if (style & DS_PRIMARY && proc) 
       if ((rets=proc(this,PA_PRIMEVENT,0))!=RET_CONT) return rets;
// ----------------------------- Focus -----------------------------
    if ((rets=fctrl->proc(fctrl,this,CA_EVENT,0))!=RET_CONT)
    {  if ((style & DS_NOTIFY) && fctrl->notify && proc)
          rets=proc(this,PA_NOTIFY,mem->Focus);
       return rets;
    }

// --------------------------- Dialog ------------------------------
    if (style & DS_ARROWS)		// Arrows to switch controls
    {  switch(keyLong)
       {  case K_DNARROW:
          case K_RTARROW:
             keyLong=K_PGDN;
	     break;
	  case K_UPARROW:
	  case K_LTARROW:
             keyLong=K_PGUP;
	     break;
       } 
    }   

    switch (keyLong)
    {  case K_ENTER:
          mem->Focus=ID_OK;
          return RET_DEFEXIT;
       case K_ESC:
       case K_CTRL_C:
       case K_TIMEOUT:
          mem->Focus=ID_CANCEL;
	  return RET_DEFEXIT;
       case K_PGUP:			// Previous control
          {  int n=mem->Focus;
	     do
	     {  if (n>0) n--;
	        else
		{  n=1;                      // ctrl[0] always presents
		   while ((ctrl+n++)->proc);
		   n-=2;
		}
	     } while (n!=mem->Focus && (ctrl+n)->style & CS_DISABLED);
   	     if (n!=mem->Focus) SetFocus(n);
	  } return RET_DONE;
       case K_PGDN:			// Next control
       case K_TAB:
	  {  int n=mem->Focus;
	     do
	     {  if (!(ctrl+ ++n)->proc) n=0;
	     } while (n!=mem->Focus && (ctrl+n)->style & CS_DISABLED);
   	     if (n!=mem->Focus) SetFocus(n);
	  } return RET_DONE; 
    } // switch
// ----------------------- proc() dispatch ------------------------
    if (style & DS_EVENTS && proc) 
       if ((rets=proc(this,PA_EVENT,0))!=RET_CONT) return rets;
// ----------------------------------------------------------------
   //beep();
   return RET_DONE;
}    

int	DIALOG::SetFocus(int n)
{  int	      old=mem->Focus;
   CONTROL *  fctrl=ctrl+old;
   
   fctrl->proc(fctrl, this, CA_DRAW, 0);
   if (style & DS_NOTIFY && proc)		// Kill Focus message
   {  fctrl->notify=CN_KILLFOCUS;
      proc(this, PA_NOTIFY, old);
   }
   mem->Focus=n;
   fctrl=ctrl+n;
   fctrl->proc(fctrl, this, CA_DRAWFOCUS, 0);
   if (style & DS_NOTIFY && proc)		// Focus message
   {  fctrl->notify=CN_FOCUS;
      proc(this, PA_NOTIFY, n);
   }
   return old;
}   

// Actions & return values:
// CA_DRAW 		n/a
// CA_DRAWFOCUS		n/a
// CA_EVENT		RET_xxx values depending of result:
//                        RET_DONE - dispatched
//  			  RET_CONT - not dispatched
//			  etc.

ulong GenControl(CONTROL *th, void * parent, int action, ulong param)
{  WINOUT	Cwin;
   ulong	rets=RET_CONT;

#define parent ((DIALOG*)parent)

   switch (th->type & CT_GENTYPE)
   {  case CT_STATIC:
         if (action==CA_DRAW)
         {  Cwin.New(&Defs);
            Cwin.RelNew(th);
            bAttr(Schemes[parent->scheme]->text[1]);
//Gotoxy(0,0);printw("th->h=%d",th->h);getch(); Cwin.at();
            Cwin.pszz(th->szzText);
	 }
	 else
	    return RET_CONT;
	 break;
      case CT_BUTTON:
         if (action==CA_DRAW || action==CA_DRAWFOCUS)
	 {  if (action==CA_DRAW)
	       bAttr(Schemes[parent->scheme]->button[th->style&CS_DISABLED?0:1]);
	    else
	       bAttr(Schemes[parent->scheme]->button[2]);
            Cwin.New(&Defs);
            Cwin.RelNew(th);
            {  int width=th->w;
               if (th->type==(CT_BUTTON|CT_MENULIKE)) 
	       {  Cwin.pc(th->val?'x':' ');
	          Cwin.pc(' ');
		  width-=2;
	       }
	       Cwin.psfd(th->szzText,width);
	       bAttr(Schemes[parent->scheme]->text[1]);
	       Cursor(0);
	    }
	 }
	 else if (action==CA_EVENT && keyLong==K_ENTER)
         {  th->notify=CN_CLICKED;
            return RET_EXIT;
         }
	 break;
      case CT_CHECKBOX:
         switch (action)
	 {  case CA_EVENT:
	       if (keyLong!=K_SPACE) return RET_CONT;
            case CA_SET:
	       th->val= action==CA_EVENT ? !th->val : param;
               rets=RET_DONE;
	    case CA_DRAW:
	       bAttr(Schemes[parent->scheme]->text[th->style&CS_DISABLED?0:1]);
               Cwin.New(&Defs);
               Cwin.RelNew(th);
	       Cwin.pc('[');
	       Cwin.pc(th->val?'x':' ');
	       Cwin.pc(']');
	       if (th->szzText)
	       {  Cwin.pc(' ');
	          Cwin.pszz(th->szzText);
	       }
	       if (action!=CA_EVENT) break;
	    case CA_DRAWFOCUS:
               Cwin.New(&Defs);
               Cwin.RelNew(th);
	       Cwin.at(0,1);
	       Cursor(1);
         } // switch
	 break;
      case CT_GROUP:
         if (!(th->style&CS_HIDDEN) && action==CA_DRAW)
	 {  Cwin.New(&Defs);
            Cwin.RelNew(th);
	    bAttr(Schemes[parent->scheme]->frame);
	    Cwin.fill(FT_SINGLE);
            if (th->szzText)
	    {  Cwin.at(0,2);
               Cwin.pc(' ');
	       Cwin.pszz(th->szzText);
	       Cwin.pc(' ');
	    }
	 }
	 break;
      case CT_SEPARATOR:
         if (action==CA_DRAW)
         {  Defs.at(th->l,0);
            bAttr(Schemes[parent->scheme]->frame);
            {  register char c=Chars[10];
	       register char n=parent->w;
	       while (n--) Defs.pc(c);
	    }
	 }
	 else
	    return RET_CONT;
	 break;
  
   } // switch
   return	rets;
}
#undef parent

ulong RadioBoxControl(CONTROL *th, void * parent, int action, ulong param)
{  WINOUT	Cwin;
   ulong	rets=RET_CONT;
   int		index;
   int		first;


#define parent ((DIALOG*)parent)
//#define param  (*((RBSETCLASS*)&param))   /* CA_SET argument */

  switch (action)
  {   case CA_EVENT:
        if (keyLong!=K_SPACE) return RET_CONT;
      case CA_SET:
         if (th->val)
         {  th->notify=CN_CLICKED;
	    return RET_DONE;
	 }
// Set index & first
         if (action==CA_SET) index=param;
         else index=parent->mem->Focus;
         first=index-1;
         {  CONTROL * ptr=parent->ctrl+first;
            while ((first--)>=0 && (ptr--)->proc==RadioBoxControl);
         }
         first+=2;
// Reset all Radio Buttons (draw IN)
         {  CONTROL * ptr=parent->ctrl+first;
            do
	    {   int temp=ptr->val;
	        ptr->val=0;
		if (temp && action==CA_EVENT)
		   ptr->proc(ptr,parent,CA_DRAW,0);  // recursive !
	    } while ((ptr++)->proc==RadioBoxControl);
	 }
// Set current
         th->val=1;
         th->notify=CN_CLICKED;
	 rets=RET_DONE;
// And after - draw it
      case CA_DRAW:
         bAttr(Schemes[parent->scheme]->text[th->style&CS_DISABLED?0:1]);
         Cwin.New(&Defs);
         Cwin.RelNew(th);
         Cwin.pc('(');
         Cwin.pc(th->val?'o':' ');
         Cwin.pc(')');
	 if (th->szzText)
         {  Cwin.pc(' ');
	    Cwin.pszz(th->szzText);
	 }
         if (action!=CA_EVENT) break;	// Draw focus in event dispatching
      case CA_DRAWFOCUS:
         Cwin.New(&Defs);
         Cwin.RelNew(th);
         Cwin.at(0,1);
         Cursor(1);
         break;
   }
   return rets;
}
#undef param
#undef parent

ulong ListBoxControl(CONTROL *th, void * parent, int action, ulong param)
{  WINOUT	Cwin;
   ulong	rets;
   int		dirty = 0;

#define parent ((DIALOG*)parent)
#define list   (*((LISTBOX*)th->mem))

   if (parent->mem->fFList) rets=RET_DONE;
   else rets=RET_CONT;

  switch (action)
  {   case CA_EVENT:
         if (parent->mem->fFList)
	    switch (keyLong)
	    {  case K_ENTER:
	          {  CONTROL * ptr=parent->ctrl+parent->mem->Focus;
		     ptr->proc(ptr,parent,CA_SET|CAS_NODRAW,th->val);
		  }
	       case K_ESC:
	          parent->mem->fFList=0;
	          return RET_REDRAW;
	    }
         switch (keyLong)
 	 {  case K_UPARROW:
	       if (th->val>0)
	       {  th->val--;
	          dirty=1;
	       }
	       break;
	    case K_DNARROW:
	       if (th->val < (unsigned)list.pcs-1)
	       {  th->val++;
	          dirty=1;
	       }
               break;
	    default:
	       return rets;
	 }
         rets=RET_DONE;
      case CA_SET:
         if (action==CA_SET && param<(unsigned)list.pcs) 
	 {  th->val=param;
	 dirty=1;
	 }
	 if (! dirty) return rets;
         th->notify=CN_CLICKED;
      case CA_DRAW:
         Cwin.New(&Defs);
         Cwin.RelNew(th);
         bAttr(Schemes[parent->scheme]->frame);
         Cwin.fill(FT_SINGLE);
         Cwin.at(0,2);
         if (th->szzText)
         {  Cwin.pc(' ');
            Cwin.pszz(th->szzText);
            Cwin.pc(' ');
	 }
	 Cwin.lin++;
	 Cwin.col++;
	 Cwin.lins-=2;
	 Cwin.cols-=2;
         Cwin.at(0,0);

         if (th->val<(unsigned)list.disp) 
		list.disp=th->val;
	 if (th->val>=(unsigned)(list.disp+Cwin.lins))
		list.disp=th->val-Cwin.lins+1;

         {  char ** llstr=list.array+list.disp;
	    int	    lins=list.pcs-list.disp; 	
	    int     cnt=Cwin.lins<=lins ? Cwin.lins:lins;
	    for (int i=0;i<cnt;i++)
	    {  bAttr(Schemes[parent->scheme]->lst[(unsigned)i+list.disp == th->val?1:0]);
	       Cwin.pc(' ');
	       Cwin.psfd(*(llstr++),Cwin.cols-2);
	       Cwin.pc(' ');
 	    }
	 }
         if (action!=CA_EVENT) break;	// Draw focus in event dispatching
     case CA_DRAWFOCUS:
        Cwin.New(&Defs);
        Cwin.RelNew(th);
        Cwin.at(th->val+1-list.disp,1);
	bAttr(Schemes[parent->scheme]->lst[2]);
        Cwin.pc(' ');
        Cwin.psfd(*(list.array+th->val),th->w-4);
        Cwin.pc(' ');
	Cursor(0);
        break;
   }
   return rets;
}
#undef parent
#undef list

ulong ComboBoxControl(CONTROL *th, void * parent, int action, ulong param)
{  WINOUT	Cwin;
   ulong	rets=RET_CONT;
   int		dirty=0;           // dirty level
   int		width=th->w-2-((th->type&CBT_MASK)==CBT_DROPDN?2:0);
   char		nbuffer[8];

#define parent ((DIALOG*)parent)
#define combo  (*((COMBOX*)th->mem))
#define float_val  *((float*)(&th->val))
#define signed_val *((long*)(&th->val))

   switch (action & (~CAS_NODRAW))
   {  case CA_EVENT:
         if (!combo.buff) return rets;
         switch (keyLong & M_KEY)
         {  case K_RTARROW:
            case K_CTRL_S:
               if(combo.pos<combo.len)
               {  combo.pos++;
                  dirty=1;
               }
	       break;
            case K_LTARROW:
            case K_CTRL_D:
               if(combo.pos>0)
               {  combo.pos--;
                  dirty=1;
               }
	       break;
            case K_HOME:
//          case K_CTRL_HOME: 		// no such keystroke
               combo.pos=0;
               dirty=1;
               break;
            case K_END:
//          case K_CTRL_END: 		// no such keystroke
               combo.pos=combo.len;
               dirty=1;
               break;
            case K_BS:
               if (!(th->type & CBT_READONLY) && combo.pos>0)
               {  if (combo.pos<combo.len)
                  {  char * last =combo.buff+combo.len;
                     char * first=combo.buff+combo.pos;
                     for (char * i=first;i<last; i++) *(i-1)=*i;
                  }
                  combo.pos--;
                  combo.len--;
                  combo.buff[combo.len]='\0';
                  dirty=2;
               }
               break;
            case K_DEL:
            case K_CTRL_G:
      	       if (!(th->type & CBT_READONLY) && combo.pos<combo.len)
               {  if ((combo.pos+1)<combo.len)
                  {  char * last =&combo.buff[combo.len];
                     char * first=&combo.buff[combo.pos+1];
                     for (char * i=first;i<last; i++) *(i-1)=*i;
                  }
                  combo.len--;
                  combo.buff[combo.len]='\0';
                  dirty=2;
               }
               break;
            case K_CTRL_Y:	// delete line
               if (!(th->type & CBT_READONLY))
	       {  combo.pos=0;
                  combo.len=0;
                  combo.buff[combo.len]='\0';
                  dirty=2;
	       }
               break;
            case K_CTRL_K: 	// delete line after cursor
               if (!(th->type & CBT_READONLY))
	       {  combo.len=combo.pos;
                  combo.buff[combo.len]='\0';
                  dirty=2;
	       }
               break;
//          case K_CTRL_LTARROW: 	// no such keystroke
            case K_CTRL_A:
//    Set to previous word END
// 1. Find space
// 2. Find first non-space symbol
               {  int i=combo.pos;
                  while (i>0 && combo.buff[--i]!=' ');
                  while (i>0 && combo.buff[--i]==' ');
                  combo.pos=i;
                  dirty=1;
                  break;
               }
//          case K_CTRL_RTARROW: 	// no such keystroke
            case K_CTRL_F:
//    Set to next word BEGIN
// 1. Find space
// 2. Find first non-space symbol
               {  int i=combo.pos;
                  while (i<combo.len && combo.buff[++i]!=' ');
                  while (i<combo.len && combo.buff[++i]==' ');
                  combo.pos=i;
                  dirty=1;
                  break;
               }
            case K_CTRL_BS:
            case K_CTRL_W:
//   Delete left (or current) word
//  1. Find left letter:
//		a. findleft nospace (or BOL)
//		b. findleft space (or BOL)
//  2. Find right letter
//		a. findright space (or EOL)
//  3. Delete [right_letter;left_letter)
//
               if (!(th->type & CBT_READONLY))
               {  int beg,end,len;
                  int i=combo.pos;
                  while (i>0 && combo.buff[--i]==' ');	// for BS policy
                  while (i>0 && combo.buff[--i]!=' ');
                  beg=i;
                  while (i<combo.len && combo.buff[++i]!=' ');
                  end=i;
                  len=end-beg;
                  if (end<combo.len)
                  {  char * last =&combo.buff[combo.len];
                     char * first=&combo.buff[end];
                     for(char * ii=first; ii<last; ii++) *(ii-len)=*ii;
                  }
                  combo.pos=beg;
                  combo.len-=len;
                  combo.buff[combo.len]='\0';
                  dirty=2;
               }
                  break;
            case K_CTRL_T:
//   Delete right (or current) word
//  1. Find right letter:
//	a. findrigth space (or EOL)
//  2. Find left letter
//	a. findleft space (or BOL)
//  3. Delete [right_letter;left_letter)
//
               if (!(th->type & CBT_READONLY))
               {  int beg,end,len;
                  int i=combo.pos;
                  while (i<combo.len && combo.buff[++i]!=' ');
                  end=i;
                  while (i>0 && combo.buff[--i]!=' ');
                  beg=i;
                  len=end-beg;
                  if (end<combo.len)
                  {  char * last =&combo.buff[combo.len];
                     char * first=&combo.buff[end];
                     for(char * ii=first; ii<last; ii++) *(ii-len)=*ii;
                  }
                  combo.pos=beg;
                  combo.len-=len;
                  combo.buff[combo.len]='\0';
                  dirty=2;
               }
	       break;
            case K_UPARROW:     // Previous (or current) item
               if ((th->type&CBT_MASK)==CBT_SIMPLE && combo.lst.pcs)
	       {  if (!strcmp(combo.buff,combo.lst.array[th->val]) &&
	               th->val>0) th->val--;
		  dirty=3;
	       }
               if ((th->type&CBT_MASK)==CBT_COUNT)
               {  if (th->type & CBT_FLOAT)
                  {  float_val=atof(combo.buff)+combo.step;
                     if (!(th->type & CBT_SIGNED) && float_val<0) float_val=0;
                     if (float_val<combo.step && float_val>-combo.step)
                        float_val=0; // fixing rounding error
                  }
                  else
                  {  signed_val=(long)(atol(combo.buff)+combo.step);
                     if (!(th->type & CBT_SIGNED) && signed_val<0) signed_val=0;
                  }
                  dirty=3;
               }
               if ((th->type&CBT_MASK)!=CBT_DROPDN) break;
	    case K_DNARROW:     // Next (or current) item
               if ((th->type&CBT_MASK)==CBT_SIMPLE && combo.lst.pcs)
	       {  if (!strcmp(combo.buff,combo.lst.array[th->val]) &&
	               (th->val+1)<(unsigned)combo.lst.pcs) th->val++;
		  dirty=3;
	       }
               if ((th->type&CBT_MASK)==CBT_COUNT)
               {  if (th->type & CBT_FLOAT)
                  {  float_val=atof(combo.buff)-combo.step;
                     if (!(th->type & CBT_SIGNED) && float_val<0) float_val=0;
                     if (float_val<combo.step && float_val>-combo.step)
                        float_val=0; // fixing rounding error
                  }
                  else
                  {  signed_val=(long)(atol(combo.buff)-combo.step);
                     if (!(th->type & CBT_SIGNED) && signed_val<0) signed_val=0;
                  }
                  dirty=3;
               }
               if ((th->type&CBT_MASK)==CBT_DROPDN && combo.lst.pcs)
	       {  if (!strcmp(combo.buff,combo.lst.array[th->val]))
	             rets=RET_REDO;
		  else rets=RET_DONE;
		  *(parent->mem->FList)=combo.lst;
		  {  CONTROL * ptr=parent->mem->FLctrl;
		     ptr->l=th->l;
		     ptr->c=th->c-1;
		     ptr->h=combo.h;
		     ptr->w=th->w+2;
		     ptr->style=CS_EXTERN;
		     ptr->szzText=0;
		     ptr->limit=combo.lst.pcs;
		     ptr->mem=parent->mem->FList;
		     ptr->val=th->val;
		     parent->mem->fDrawCur=1;
		     parent->mem->fFList=1;
		     return rets;
		  }
	       }
	       break;     
            default: 
               if ((th->type & CBT_READONLY) != 0) return RET_CONT;
               if (
(th->type & CBT_NUMERIC && keyLong>0x2f && keyLong<0x3a)                   ||
(th->type & CBT_FLOAT && ((keyLong>0x2f && keyLong<0x3a) || keyLong=='.')) ||
(th->type & CBT_SIGNED && keyLong=='-')					   ||
(!(th->type & (CBT_NUMERIC|CBT_FLOAT)) && keyLong>31 && keyLong<256)
                  )
               {  if (combo.len < th->limit)
                  {  
// Insert/Replace modes support (DISABLED)
//		     if (!combo.fInsert && combo.pos < combo.len)
                     {  char * last =combo.buff+combo.len; 
                        char * first=combo.buff+combo.pos; 
                        for (char * i=last;i>first; i--) *i=*(i-1); 
                     } 
                     combo.buff[combo.pos]=(char)keyLong; 
                     combo.pos++; 
                     combo.len++; 
                     combo.buff[combo.len]='\0'; 
                     dirty=2; 
                  } 
               } else return RET_CONT;
         }  // switch keyLong
         rets=RET_DONE;
	 if (! dirty) break;
      case CA_SET:
         if ((action & (~CAS_NODRAW))==CA_SET && !combo.lst.array) return rets;
	 if ((action & (~CAS_NODRAW))==CA_SET)
	 {  th->val=param;
	    dirty=3;
	 }
      case CA_SETTEXT:
         if ((action & (~CAS_NODRAW))!=CA_EVENT || dirty==3)
         {  char * src;
	    char * dst=combo.buff;
	    int    cnt=th->limit;

	    if ((action & (~CAS_NODRAW))==CA_SETTEXT) src=(char*)param;
	    else
	       if ((th->type&CBT_MASK)==CBT_COUNT)
	       {  if (th->type & CBT_FLOAT)
	             sprintf(nbuffer,"%g%c",float_val, (char)0);
		  else if (th->type & CBT_SIGNED)
		     sprintf(nbuffer,"%ld",signed_val);
		  else
		     sprintf(nbuffer,"%ld",th->val);
		  src=nbuffer;
	       }
	       else src=combo.lst.array[th->val];

	    while (cnt-- && *src) *(dst++)=*(src++);
	    *dst='\0';
	    combo.pos=0;
	    combo.disp=0;
	    combo.len=th->limit-cnt-1;
	 }
      case CA_DRAW:
	 if (combo.disp > combo.pos)
	    {  combo.disp=combo.pos;
	       dirty=2;
	    } else
               if (combo.disp+width <= combo.pos)
	       {  combo.disp=combo.pos-width+1;
	          dirty=2;
	       }
         if (dirty>1) th->notify=CN_CLICKED;
         if (action & CAS_NODRAW) return rets;
	 if (dirty>1 || action==CA_DRAW)
	 {  Cwin.New(&Defs);
	    Cwin.RelNew(th);
	    bAttr(Schemes[parent->scheme]->edit[th->style&CS_DISABLED ? 0:1]);
	    Cwin.pc(' ');

	    if ((th->type&CBT_PASSWORD) == 0) 
               Cwin.psfd(combo.buff+combo.disp,width);
            else
            {  int i, ii;
               i  = strlen(combo.buff+combo.disp);
               i  = i > width ? width : i;
               ii = width - i;
               for (; i>0; i--)   Cwin.pc('*');
               for (; ii>0; ii--) Cwin.pc(' ');
            } 

	    Cwin.pc(' ');
	    if ((th->type&CBT_MASK)==CBT_DROPDN)
	    {  Cwin.ps("[]");
	    }
	 }
	 if (action!=CA_EVENT) break;
      case CA_DRAWFOCUS:
         Cwin.New(&Defs);
	 Cwin.RelNew(th);
         {  int temp=combo.pos-combo.disp;
	    Cwin.at(0, temp+1);
	 }
	 Cursor(1);
	 break;
   }         
   return rets;
}
#undef parent
#undef combo

