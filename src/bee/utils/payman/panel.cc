// Shell
// Panel Procedure file

#include "global.h"

int	PanelProc (AREA * th, int action, ulong param)
{   switch (action)
    {   case OA_DISPEVENT:
           return RET_CONT;
	case OA_MINORUPD:
	case OA_MAJORUPD:
	   Defs.New(th);
           Attr(14,4);
	   Defs.fill();
	   Defs.ps((char*)" No panel support, yet.");
	   return 0;
	default:
	   return RET_CONT;
    }
}
