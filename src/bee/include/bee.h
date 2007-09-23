/* $RuOBSD: bee.h,v 1.11 2007/09/15 11:03:31 shadow Exp $ */

#ifndef __BEE_H__
#define __BEE_H__
/*
 * Global project header & header of project library
 */


#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SUCCESS	   0

#define SWAP(a,b) ((a)^=(b)^=(a)^=(b))

typedef int (*cmpfunc_t)(void * one, void * two);

__BEGIN_DECLS

char * next_token  (char ** ptr, char * delim);
char * strtrim(char * ptr, char * delim);
char * alloc_token (char ** ptr, char * delim);


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

// Swap two items, Error (-1).
int    da_swap    (int * cnt, void * array, int size, int ind1, int ind2);
// Sort items with external compare function, Error (-1).
int    da_bsort   (int * cnt, void * array, int size, cmpfunc_t func);
// Reverse items order, Error (-1).
int    da_reverse (int * cnt, void * array, int size);

int    memswap(void * ptr1, void * ptr2, int len);

__END_DECLS

#endif /* __BEE_H__ */

