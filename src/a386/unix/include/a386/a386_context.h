/**********************************************************************
unix/a386_types.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.
**********************************************************************/

#ifndef _A386_UNIX_A386_CONTEXT_H
#define _A386_UNIX_A386_CONTEXT_H

#include <a386/a386_types.h>
#include <a386/sysdep.h>

struct a386_context
{
  char regs[_A386_SIZEOF_REGS];
  int kernel_mode;
  int interrupts_enabled;
};

extern void _a386_context_get_a386 (struct a386_context *, int);
extern int  _a386_context_set_a386 (struct a386_context *);

#endif /* _A386_UNIX_A386_CONTEXT_H */
