/* $RuOBSD: beetraff.h,v 1.6 2004/05/08 17:18:56 shadow Exp $ */

#ifndef __BEETRAFF_H__
#define __BEETRAFF_H__

#include <sys/types.h>

#define MAX_STRLEN    1024
#define IPFSTAT_DELIM " \t\n"

#define ITMF_COUNT  1

#define OPTS "r:a:A:un:N:lf:o:"

// statistics storage structure
typedef struct
{  u_long addr;       // IP address
   u_long in;         // inbound count 
   u_long out;        // outbound count 
} accsum_t;


typedef struct
{  u_long addr;
   u_long mask;
   int    flag;
} exclitem_t;


void usage      (int rc);
int  inaddr_cmp (char * user, char * link);

#endif /* __BEETRAFF_H__ */
