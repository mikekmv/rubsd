// File Shell
// Dispatch file

#include "global.h"
#include "list.h"
#include "userview.h"
#include "da.h"

extern int EventType;

int UserViewDisp();
int HelpDisp();

int	DispEvent()
{
   Attr(8,7); Gotoxy(0, 0); uprintf("%04lx    ", keyLong & M_KEY); refresh();

   if ( (keyLong & M_KEY) == K_CTRL_L)
   {  DoRefresh = 1;
      return RET_DONE;
   }

   if (StageScr == 1) return UserViewDisp();
   if (StageScr > 1) return HelpDisp();

   switch(keyLong & M_KEY)
   {  
      case K_ENTER:
         UserView.user = (userdata_t *)da_ptr(
                  &(UserList.cnt_users),
                  &(UserList.itm_users),
                  sizeof(userdata_t), UserList.marked);
         if (UserView.user != NULL)
         {  StageScr = 1;
            keymode = 2;
            DoRefresh = 1;
            UserView.load_accs();  
         }
         return RET_DONE;  // stub
      case K_F(1):
         StageScr = 2;
         keymode = 3;
         DoRefresh = 1;
         return RET_DONE; 
      case K_F(10):
         if (MessageBox("Выход из программы\0",
	                " Вы хотите завершить сеанс ? \0",
	                MB_YESNO|MB_NEUTRAL)==ID_YES) return RET_EXIT;
         DoRefresh = 1;
         return RET_DONE;
      case K_HOME:
         if (UserList.marked > 0)
         {  UserList.last_marked = UserList.marked;
            UserList.marked = 0;
            UserList.flags |= ULF_LIGHTMOV;
            if (UserList.marked < UserList.first)
            {  UserList.first = UserList.marked;
               UserList.flags |= ULF_WINDMOV;
            }
         }
         return RET_DONE;
      case K_END:
         if (UserList.marked < (UserList.cnt_users - 1))
         {  UserList.last_marked = UserList.marked;
            UserList.marked = UserList.cnt_users - 1;
            UserList.flags |= ULF_LIGHTMOV;
            if (UserList.marked >= (UserList.first + UserList.lins))
            {  UserList.first = UserList.marked - UserList.lins + 1;
               UserList.flags |= ULF_WINDMOV;
            }
         }
         return RET_DONE;
      case K_UPARROW:
         if (UserList.marked > 0)
         {  UserList.last_marked = UserList.marked;
            UserList.marked--;
            UserList.flags |= ULF_LIGHTMOV;
            if (UserList.marked < UserList.first)
            {  UserList.first = UserList.marked;
               UserList.flags |= ULF_WINDMOV;
            }
         }
         return RET_DONE;
      case K_DNARROW:
         if (UserList.marked < (UserList.cnt_users - 1))
         {  UserList.last_marked = UserList.marked;
            UserList.marked++;
            UserList.flags |= ULF_LIGHTMOV;
            if (UserList.marked >= (UserList.first + UserList.lins))
            {  UserList.first = UserList.marked - UserList.lins + 1;
               UserList.flags |= ULF_WINDMOV;
            }
         }
         return RET_DONE;
      case K_PGUP:
         if (UserList.marked > 0)
         {  UserList.last_marked = UserList.marked;
            UserList.marked -= UserList.lins;
            if (UserList.marked < 0) UserList.marked = 0;
            UserList.first -= UserList.lins;
            if (UserList.first < 0) UserList.first = 0;
            UserList.flags |= ULF_WINDMOV;
         }
         return RET_DONE;
      case K_PGDN:
         if (UserList.marked < (UserList.cnt_users - 1))
         {  UserList.last_marked = UserList.marked;
            UserList.marked += UserList.lins;
            if (UserList.marked >= UserList.cnt_users) UserList.marked =  UserList.cnt_users - 1; 
            UserList.first += UserList.lins;
            if (UserList.first > (UserList.cnt_users - UserList.lins))
               UserList.first = UserList.cnt_users - UserList.lins;
            if (UserList.first < 0) UserList.first = 0;
            UserList.flags |= ULF_WINDMOV;
         }
         return RET_DONE;
      case K_F(2):
         UserList.sort_regname();
         UserList.flags |= ULF_WINDMOV;
         return RET_DONE; 
      case K_F(3):
         UserList.sort_ip();
         UserList.flags |= ULF_WINDMOV;
         return RET_DONE; 
      case K_F(4):
         UserList.sort_port();
         UserList.flags |= ULF_WINDMOV;
         return RET_DONE; 
      case K_F(5):
         UserList.rev_order();
         UserList.flags |= ULF_WINDMOV;
         return RET_DONE; 

   } // (switch)

   return RET_DONE;
}

int UserViewDisp()
{

   switch(keyLong & M_KEY)
   {  
      case K_F(1):
         StageScr = 3;
         keymode = 3;
         DoRefresh = 1;
         return RET_DONE; 
      case K_F(10):
      case K_ESC:
         StageScr = 0;
         keymode = 1;  
         DoRefresh = 1;
         return RET_DONE;
   }

   return RET_DONE;
}

int HelpDisp()
{
   switch(keyLong & M_KEY)
   {  
      case K_F(10):
      case K_ESC:
         StageScr -= 2;
         keymode = StageScr + 1;  
         DoRefresh = 1;
         return RET_DONE;
   }

   return RET_DONE;
}

#ifdef NEVER
//       Dispatch queue
// 1. Program specific keys / events
// 2. Current panel (panel internal hotkeys)
// 3. Command line keys

//       Returns (cycle verdict)
// RET_CONT - continue cycle
// RET_EXIT - Program exit selected
// RET_EXEC - Execute shell command (Pass command to OSshel)
// RET_TERM - Terminal resized - rearrange Areas
//   (Area procedures return values, exept RET_DONE)
// RET_DONE - Event dispatched (by Area proc), interrupt queue

int	DispEvent()
{
   static int fEnabled;
   static int fVisible=1;
   static int nLmode=1;
   static int nRmode=1;
   static int nChoice=0;
//-------------------------- Terminal ----------------------------------------
   if (keyLong==K_TERMRESIZE) 
   {  GetSizes();
      return RET_TERM;
   }
//--------------------------- Program keys ------------------------------------
   switch(keyLong & M_KEY)
   {  case K_ENTER:
         return RET_EXEC;
      case K_F(10):
         if (MessageBox("Quit program\0",
	                "  Do you really want to"
                        " \nquit Closed Commander\0",
	                MB_YESNO|MB_NEUTRAL)==ID_YES) return RET_EXIT;

         return RET_DONE;
      case K_ESC: 
         return RET_EXIT;
      case K_F(8):
         testDialog.Dialog(0);
	 return RET_DONE;
      case K_F(9):
        {  int rets;
           OptionsMenuControls[0].val=fEnabled?0:1;
           if (fVisible)  
           {  OptionsMenuControls[1].val=0;
	      LeftMenuControls[11].style&=(~CS_HIDDEN);
	      LeftMenuControls[12].style&=(~CS_HIDDEN);
	      RightMenuControls[11].style&=(~CS_HIDDEN);
	      RightMenuControls[12].style&=(~CS_HIDDEN);
	      if (fEnabled)
	      {  LeftMenuControls[11].style&=(~CS_DISABLED);
	         LeftMenuControls[12].style&=(~CS_DISABLED);
	         RightMenuControls[11].style&=(~CS_DISABLED);
	         RightMenuControls[12].style&=(~CS_DISABLED);
	      } else
	      {  LeftMenuControls[11].style|=CS_DISABLED;
	         LeftMenuControls[12].style|=CS_DISABLED;
	         RightMenuControls[11].style|=CS_DISABLED;
	         RightMenuControls[12].style|=CS_DISABLED;
	      } 
	   } else
           {  OptionsMenuControls[1].val=1;
	      LeftMenuControls[11].style|=CS_HIDDEN|CS_DISABLED;
	      LeftMenuControls[12].style|=CS_HIDDEN|CS_DISABLED;
	      RightMenuControls[11].style|=CS_HIDDEN|CS_DISABLED;
	      RightMenuControls[12].style|=CS_HIDDEN|CS_DISABLED;
	   }
           for (int i=0;i<5;i++)
	   {  RightMenuControls[i].val= i==nRmode?1:0;
	      LeftMenuControls[i].val= i==nLmode?1:0;
	      if (i<3) FileMenuControls[i].val= i==nChoice?1:0;
	   }
	   rets=PullDn.Dialog(0);
           if (!rets) return RET_DONE;
	   if (!(rets & K_MENUID))
	   {  
	      keyLong=rets;
	      return RET_REDO;
	   }
           if (rets<MI_CH1 && (rets&0x0f)<6) 
	   {  *((rets&0x10)?&nRmode:&nLmode)=rets&0x0f-1;
	      return RET_DONE;
	   }
	   if (rets>=MI_CH1 && (rets&0x0f)<3)
	   {  nChoice=rets&0x0f;
	   }
           if (rets==MI_MOUNTDISABLE) fEnabled=!fEnabled;
           if (rets==MI_MOUNTHIDE) fVisible=!fVisible;
           return RET_DONE;   
        }
   } // (switch)
//------------------------- Command line dispatch -----------------------------
  (Area[AI_CMDVIEW].proc)(&Area[AI_CMDVIEW],OA_DISPEVENT,0);

  return RET_DONE;
}
#endif
