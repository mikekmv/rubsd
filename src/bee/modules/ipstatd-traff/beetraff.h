/* $RuOBSD: beetraff.h,v 1.3 2002/10/24 15:55:44 shadow Exp $ */

#ifndef __BEETRAFF_H__
#define __BEETRAFF_H__

#define MAX_STRLEN 1024
#define IPFSTAT_DELIM " \t\n"

#define ITMF_COUNT  1


typedef struct
{  char * item;
   int    flag;
} exclitem_t;

int inaddr_cmp(char * user, char * link);

#endif /* __BEETRAFF_H__ */
