// File Shell
// Display file

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "global.h"
#include "list.h"

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
   int i;

   if (DoRefresh != 0)
   {  Attr(7, 0);
      clear();
      RefreshTitle();
      RefreshKeybar();
      Attr(7, 0);
      Gotoxy(2,0);
      
      DoRefresh = 0;

      UserList.flags = ULF_REFRESH;
   }   

   switch (StageScr)
   {  case 0:
         UserList.refresh();
         // Park cursor
         Gotoxy(ForceLins - 3, 1);
         break;
      case 1:
         if (DoRefresh != 0)
         {  Gotoxy(2, 2); Attr(7, 0);
            uprintf("  Пользователь: %s", UserList.itm_users[UserList.marked].regname);

            Gotoxy(3, 2); Attr(7, 0);
            uprintf("         Счета: ");
            if (UserList.itm_users[UserList.marked].inet_acc >= 0)
               uprintf("╧ %d (инет) ", UserList.itm_users[UserList.marked].inet_acc);
            if (UserList.itm_users[UserList.marked].intra_acc >= 0)
               uprintf("╧ %d (сеть)", UserList.itm_users[UserList.marked].intra_acc);
        
            Gotoxy(4, 2); Attr(7, 0);
            uprintf("         Хосты: ");
            if (UserList.itm_users[UserList.marked].cnt_hosts < 1)
               uprintf("-");
            else
               for (i=0; i < UserList.itm_users[UserList.marked].cnt_hosts; i++)
               {  uprintf("%s/%d", 
                  inet_ntoa( *((in_addr*)&(UserList.itm_users[UserList.marked].itm_hosts[i].addr)) ),
                  UserList.itm_users[UserList.marked].itm_hosts[i].mask );
                  if (i < (UserList.itm_users[UserList.marked].cnt_hosts - 1) )
                    uprintf(", ");  
               }  

            Gotoxy(5, 2); Attr(7, 0);
            uprintf("Почтовые ящики: ");
            if (UserList.itm_users[UserList.marked].cnt_mail < 1)
               uprintf("-");
            else
               for (i=0; i < UserList.itm_users[UserList.marked].cnt_mail; i++)
               {  uprintf("%s@%s", 
                     UserList.itm_users[UserList.marked].itm_mail[i].login,
                     UserList.itm_users[UserList.marked].itm_mail[i].domain );
                  if (i < (UserList.itm_users[UserList.marked].cnt_mail - 1) )
                    uprintf(", ");  
               }
            Gotoxy(5, 2); Attr(7, 0);
            uprintf("         Порты: ");
            if (UserList.itm_users[UserList.marked].cnt_ports < 1)
               uprintf("-");
            else
               for (i=0; i < UserList.itm_users[UserList.marked].cnt_ports; i++)
               {  uprintf("%s:%s", 
                     UserList.itm_users[UserList.marked].itm_ports[i].switch_id,
                     UserList.itm_users[UserList.marked].itm_ports[i].port );
                  if (i < (UserList.itm_users[UserList.marked].cnt_ports - 1) )
                    uprintf(", ");  
               }
         
         }
   }

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
