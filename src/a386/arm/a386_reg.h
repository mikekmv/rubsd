/**********************************************************************
arm/a386_reg.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

a386_get/set_sp()		Get/set stack pointer.
a386_get/set_ip()		Get/set instruction pointer.
**********************************************************************/

#ifndef _ARM_A386_REG_H
#define _ARM_A386_REG_H

extern inline a386_address_t
a386_get_sp (void)
{
#if 0
  /* this generates better code, but also lots of warnings */
  register a386_address_t sp asm ("%sp");
  return sp;
#else
  a386_address_t sp;
  asm ("mov %0, %%sp" : "=r" (sp));
  return sp;
#endif
}

extern inline void
a386_set_sp (a386_address_t newsp)
{
  asm volatile ("ldr sp, %0" : : "m" (newsp));
#if 0
  register a386_address_t sp asm ("sp");
  sp = newsp;
#endif
}

static inline void
a386_set_ip (a386_address_t ip)
{
  asm volatile ("mov pc, %0\n"
		: : "r" (ip));
}

static inline a386_result_t
a386_get_result (void)
{
  register a386_result_t result asm ("%r0");
  return result;
}

static inline void
a386_set_result (a386_result_t result)
{
  asm volatile ("mov r0, %0" : : "r" (result));
}

#if 0
static inline a386_arg_t
a386_get_arg (int n)
{
  a386_arg_t data;

#define arg(m) \
  if (n == m) \
    asm volatile ("mov %0, r" #m : "=r" (data)); \
  else

  arg (0)
  arg (1)
  arg (2)
  arg (3)
  arg (4)
    {
      data = -1;
    }

#undef arg

  return data;
}

static inline void
a386_set_arg (int n, a386_arg_t data)
{
#define arg(m) \
  if (n == m) \
    asm volatile ("mov r" #m ", %0" : : "r" (data)); \
  else

  arg (0)
  arg (1)
  arg (2)
  arg (3)
  arg (4)
    {
      /* n > 4 */
    }

#undef arg
}
#endif

#endif /* _ARM_A386_H */
