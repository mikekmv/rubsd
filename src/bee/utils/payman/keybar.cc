// Shell
// Keybar procedure file

#include "global.h"

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
	  
