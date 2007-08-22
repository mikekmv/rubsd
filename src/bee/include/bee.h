/* $RuOBSD: bee.h,v 1.6 2004/05/08 15:35:54 shadow Exp $ */

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

// Forward declarations
typedef struct _acc_t		acc_t;
typedef struct _acc_t_old	acc_t_old;
typedef struct _accbase_t	accbase_t;
typedef struct _is_data_t	is_data_t;
typedef struct _logrec_t	logrec_t;
typedef struct _logbase_t	logbase_t;
typedef struct _command_t	command_t;
typedef struct _resource_t	resource_t;

// Generic types
typedef double        money_t;      // Money format (signed) 
typedef unsigned long value_t;      // Resource count value

struct _is_data_t
{  int            res_id;
   int            user_id;
   value_t        value;
   int            proto_id;
   struct in_addr host;
   int            proto2;
   long           reserv[2]; 
};

typedef int (*cmpfunc_t)(void * one, void * two);

__BEGIN_DECLS

char * next_token  (char ** ptr, char * delim);
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

