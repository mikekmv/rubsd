/* $Bee$ */

#ifndef __IPC_H__
#define __IPC_H__

#include <stdio.h>
#include <stdarg.h>

#define BEE_SERVICE	49160              /* default server port */
#define BEE_ADDR	"192.168.111.254"  /* default server address*/	

#define LINK_DOWN   (-1)  // link closed
#define LINK_ERROR  (-2)  // no fatal error

// Return codes
#define RET_SUCCESS   000
#define RET_STR       001
#define RET_INT       002
#define RET_BIN       003
#define RET_COMMENT   100
#define ERR_ACCESS    400
#define ERR_INVCMD    401
#define ERR_NOTIMPL   402
#define ERR_ARGCOUNT  403
#define ERR_INVARG    404
#define ERR_IOERROR   405
#define ERR_NOACC     406
#define ERR_SYSTEM    407

#define WAIT_ERROR    (-1)

typedef struct
{  int  fd;
   int  Local;   // ignore external requests (server)
   int  fStdio;  // don't use sockets (server debug mode)
} link_t;

int  link_request(link_t * ld, char * host, int service);
int  link_wait   (link_t * ld, int service);
int  link_close  (link_t * ld);

int link_gets (link_t * ld, char * buf, int len);
int link_puts (link_t * ld, char * str, ...);
int link_chkin(link_t * ld);

int answait (link_t * ld, int code, char * buf, int sz, char ** msg);

#endif /* __IPC_H__ */

