/* $RuOBSD: beerep.h,v 1.8 2005/04/20 12:18:20 shadow Exp $*/
#ifndef __BEEREP_H__
#define __BEEREP_H__

// offset flags
#define OFLAG_FIRST  1
#define OFLAG_LAST   2

// flags
#define FLAG_SUMCOUNT      1
#define FLAG_SUMMONEY      2
#define FLAG_DIRGROUP      4
#define FLAG_GROUPFAILS    8

#define IDXMARKER "bee0"

typedef struct
{  time_t   time_from;
   time_t   time_to;
   int      ind_from;
   int      ind_to;
} indexes_t;


typedef struct
{  int     accno;
   time_t  from;
   time_t  to;
   int     res;
   char  * title;
   char  * fields;
   int     flags;
   char  * tabopts;
   char  * cellopts;
   char  * headopts;
   char  * bodyopts;
} tformat_t;

typedef struct
{  char     marker[4];
   time_t   first;
} idxhead_t;

typedef struct
{  int          accno;
   char       * descr;
   long long    count_in;
   long long    count_out;
   long double  money_in;
   long double  money_out;
   long double  pays;
} acclist_t;


int     print_table  (tformat_t * tform, u_int64_t * sc, long double * sm, int ind);
int     print_record (logrec_t * rec, u_int64_t count, long double sum, tformat_t * tform);
int     print_line_record(u_int64_t count_in, u_int64_t count_out, long double sum_in,
            long double sum_out, tformat_t * tform);
time_t  parse_time(char * str);

void    usage();

char * strtime(time_t utc);

#endif /* __BEEREP_H__ */

