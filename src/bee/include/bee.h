/* $RuOBSD: bee.h,v 1.4 2003/07/24 04:27:57 shadow Exp $ */

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

// Forward declarations
typedef struct _acc_t		acc_t;
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


__BEGIN_DECLS

char * next_token  (char ** ptr, char * delim);
char * alloc_token (char ** ptr, char * delim);

__END_DECLS

#endif /* __BEE_H__ */
