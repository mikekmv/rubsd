/* $oganer: keepd.h,v 1.3 2003/02/15 21:19:50 shadow Exp $ */

#ifndef __IPCHK_H__
#define __IPCHK_H__

#include <sys/types.h>
#include <netinet/in.h>

/*   Switches
   >> rules file               -f
   >> verboze mode             -v
   >> help                     -?
*/

#define OPTS "f:h:v?"

#define ACCESS_GRANTED    1
#define ACCESS_DENIED     0

typedef union
{  struct in_addr  inaddr;
   u_long          dword;
   u_char          byte[4];
} inaddr_t;

void usage();
int setrealpath(char * file, char ** ptr);
int ipcmp(in_addr_t host, in_addr_t rule, int mask);

#endif /*__IPCHK_H__*/
