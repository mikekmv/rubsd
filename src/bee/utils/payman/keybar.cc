// Shell
// Keybar procedure file

#include "global.h"




int keymode = 0;

char * keybars[5][10]=
{  { "","","","","","","","","","" },
   { "Помощь","Имя","IP","Порт","Реверс","","","","Логин","Выход"},
   { "Помощь","","","","","","","","","Список"},
   { "","","","","","","","","","Назад"},
   { "Help","Menu","View","Edit","Copy","Move","MkDir","Delete","PullDn","Quit"}
};



int RefreshKeybar()
{  int i;

   Gotoxy(ForceLins-1, 0);

   Attr(7,0); Putch('1');
   Attr(0,7); uprintf("%-6s", keybars[keymode][0]);

   for (i=1; i<9; i++)
   {  Attr(7,0); uprintf(" %d", i+1);
      Attr(0,7); uprintf("%-6s", keybars[keymode][i]);
   }

   Attr(7,0); Puts(" 10");
   Attr(0,7); uprintf("%-6s", keybars[keymode][9]);

   return 0;
}


int	KeyProc(AREA * th, int action, ulong param)
{   switch (action)
    {   case OA_DISPEVENT:
           return RET_CONT;
	case OA_MINORUPD:
	case OA_MAJORUPD:
	   Defs.New(th);
	   Attr(7,0);
//	   Defs.ps("xxx");


	   Defs.pszz(
"\0c\7\0""1\0c\0\6Help  "
"\0c\7\0 2\0c\0\6Menu  "
"\0c\7\0 3\0c\0\6View  "
"\0c\7\0 4\0c\0\6Edit  "
"\0c\7\0 5\0c\0\6Copy  "
"\0c\7\0 6\0c\0\6RenMov"
"\0c\7\0 7\0c\0\6MkDir "
"\0c\7\0 8\0c\0\6Delete"
"\0c\7\0 9\0c\0\6PullDn"
"\0c\7\0 10\0c\0\6Quit  "
"\0c\7\0\0");
           return 0;
	default:
	   return RET_CONT;
    }
}	   
	  
