/* $RuOBSD: beetraff.h,v 1.7 2009/04/09 06:56:27 shadow Exp $ */

#ifndef __BEETRAFF_H__
#define __BEETRAFF_H__

#include <sys/types.h>

#define MAX_STRLEN    1024
#define IPFSTAT_DELIM " \t\n"

#define ITMF_COUNT  1

#define OPTS "r:a:A:un:N:lf:o:ch:dDL"

// statistics storage structure
typedef struct
{  u_int  addr;       // IP address
   u_int  in;         // inbound count 
   u_int  out;        // outbound count 
   u_int  accno;      // account number
} accsum_t;


typedef struct
{  u_int  addr;
   u_int  mask;
   int    flag;
} exclitem_t;


void usage      (int rc);
int  inaddr_cmp (char * user, char * link);

#endif /* __BEETRAFF_H__ */
