/* $oganer: da.h,v 1.1 2003/06/09 12:27:03 tm Exp $ */

#ifndef _DA_H
#define _DA_H

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

#endif /* _DA_H */
