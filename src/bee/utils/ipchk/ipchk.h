/* $RuOBSD: ipchk.h,v 1.3 2007/09/25 14:25:18 shadow Exp $ */

#ifndef __IPCHK_H__
#define __IPCHK_H__

#include <sys/types.h>
#include <netinet/in.h>

#define OPTS "f:v?sFh"

#define ACCESS_GRANTED    1
#define ACCESS_DENIED     0

typedef union
{  struct in_addr  inaddr;
   u_int           dword;
   u_char          byte[4];
} inaddr_t;

void usage();
int setrealpath(char * file, char ** ptr);
int ipcmp(in_addr_t host, in_addr_t rule, int mask);

#endif /*__IPCHK_H__*/
