/* $RuOBSD: beerep.h,v 1.2 2004/02/02 09:12:49 shadow Exp $*/
#ifndef __BEEREP_H__
#define __BEEREP_H__

// flags
#define FLAG_SUMCOUNT      1
#define FLAG_SUMMONEY      2
#define FLAG_DIRGROUP      4
#define FLAG_GROUPFAILS    8

typedef struct
{  int     accno;
   time_t  from;
   time_t  to;
   int     res;
   char  * title;
   char  * fields;
   int     flags;
} tformat_t;

int     print_table  (tformat_t * tform, u_int64_t * sc, long double * sm);
int     print_record (logrec_t * rec, u_int64_t count, long double sum, tformat_t * tform);

time_t  parse_time(char * str);

char * strtime(time_t utc);

#endif /* __BEEREP_H__ */

