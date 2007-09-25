// File Shell
// WaitEvent file

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>

#include "global.h"

int		eventType; 
uint		keyLong;

int	WaitEvent()
{  fd_set          fdset;
   int             rc;
   struct timeval  tv = {180, 0};

   refresh();

   if (eventType == EV_USER && keyLong == K_TIMEOUT) return 0;

   FD_ZERO(&fdset);
   FD_SET(fileno(stdin), &fdset);

   rc = select(fileno(stdin)+1, &fdset, NULL, NULL, &tv);

   if (rc < 0)
      syslog(LOG_ERR, "WaitEvent(select): %m");

   if (rc == 0)
   {  //syslog(LOG_ERR, "WaitEvent(select): timeout");
      eventType = EV_USER;
      keyLong   = K_TIMEOUT;
      return 0;
   }
  
   keyLong=GetKey();			// No dinamic events yet ...
   eventType=EV_USER;
//   fMinorUpdate=1;
   return 0;
}

