/* $oganer: misc.h,v 1.1 2002/11/18 02:58:35 shadow Exp $ */

#ifndef __MISC_H__
#define __MISC_H__

#include <sys/time.h>

////////////////////////////////
//   dynamic array functions  //
////////////////////////////////

// Empty dynamic array. Sucess 0, Error (-1).
int    da_empty(int * cnt, void * array, int size);
// Adds new entry to given ind (or end if ind=-1), zero fill, return its pointer. Error NULL.
void * da_new  (int * cnt, void * array, int size, int ind);
// Adds new entry to given ind (or end if ind=-1) & copy from src. new member no or (-1) on error.
int    da_ins	(int * cnt, void * array, int size, int ind, void * src);
// Removes given entry to dst (if not NULL). Sucess 0, Error (-1).
int    da_rm   (int * cnt, void * array, int size, int ind, void * dst);
// Returns given entry pointer. Error NULL.
void * da_ptr	(int * cnt, void * array, int size, int ind);
// Copies src to given entry. Sucess 0, Error (-1).
int    da_put	(int * cnt, void * array, int size, int ind, void * src);
// Copies given entry to dst. Sucess 0, Error (-1).
int    da_get (int * cnt, void * array, int size, int ind, void * dst);

/* * * * * * * * * * *\ 
 *  string functions *
\* * * * * * * * * * */

// Get string next token function (destructive version)
char * next_token(char ** ptr, char * delim);
// Make new allocated string copy
char * strnalloc(char * str, int len);

/* * * * * * * * * * *\ 
 *   time functions  *
\* * * * * * * * * * */

#ifdef _NEED_TIMEGM_PROTOTYPE
// timegm() prototype
time_t timegm(struct tm * tm);
#endif /* _NEED_TIMEGM_PROTOTYPE */

void   get_time     (time_t * ptime, int * pslice);
time_t shift_time   (time_t utc);
void   newget_time  (struct timeval * tv);

#endif /* __MISC_H__ */

