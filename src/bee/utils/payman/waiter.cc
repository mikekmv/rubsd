// File Shell
// WaitEvent file

#include "global.h"

int		eventType; 
ulong		keyLong;

int	WaitEvent()
{
    keyLong=GetKey();			// No dinamic events yet ...
    eventType=EV_USER;
//    fMinorUpdate=1;
    return 0;
}

