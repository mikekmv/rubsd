#ifndef _A386_UNIX_DEBUG_H
#define _A386_UNIX_DEBUG_H

//#include "os/syscalls-linux.h"
#include <a386/a386_reg.h>

extern void a386_putchar (char c);
extern int a386_getchar (void);
extern void a386_puts (const char *s);
extern void a386_printf (const char *format, ...);
extern void a386_dump_stack (int n);

#if 0
static inline void
a386_putchar (char c)
{
  unix_write (2, &c, 1);
}

static inline int
a386_getchar (void)
{
  unsigned char c;
  unix_read (0, &c, 1);
  return (int)c;
}
#endif

extern inline void
a386_dump_stack (int n)
{
  int i;

  n *= 7;

  for (i = 0; i < n; i++)
    {
      if (i % 7 == 0)
	a386_printf ("%08lx: ", &((long *)a386_get_sp ())[i]);
      a386_printf ("%08lx ", ((long *)a386_get_sp ())[i]);
      if (i % 7 == 6)
	a386_putchar ('\n');
    }
}

#endif /* _A386_UNIX_DEBUG_H */
