#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>

int log_open  ();
int log_close ();
int log_write (char * format, ...);

#endif /*__LOG_H__*/

