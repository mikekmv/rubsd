#ifndef __BEETALK_H__
#define __BEETALK_H__

#include <stdarg.h>


// Connect bee & initialize session
int bee_enter();

// Send command
int bee_send    (char * cmd, char * argfmt, ...);
// Receive answer
int bee_recv    (int code, char * buf, int sz, char ** msg, int * last);

// Send/Recv "update" command
int bee_update     ();
// Send/Recv "intraupdate" command
int bee_intraupdate();

// Terminate bee session
int bee_leave();

#endif /* __BEETALK_H__ */

