/**********************************************************************
i386/a386_reg.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

a386_get/set_sp()		Get/set stack pointer.
a386_get/set_ip()		Get/set instruction pointer.
**********************************************************************/

#ifndef _A386_I386_A386_REG_H
#define _A386_I386_A386_REG_H

extern inline a386_address_t
a386_get_sp (void)
{
  register a386_address_t sp asm ("%esp");
  return sp;
}

extern inline void
a386_set_sp (a386_address_t sp)
{
  asm volatile ("movl %0,%%esp" : : "mr" (sp) : "esp");
}

extern inline a386_result_t
a386_get_result (void)
{
  register a386_result_t result asm ("%eax");
  return result;
}

extern inline void
a386_set_result (a386_result_t result)
{
  asm volatile ("movl %0,%%eax" : : "mr" (result) : "eax");
}

extern inline void
a386_set_ip (a386_address_t ip)
{
  asm volatile ("jmp *%0\n"
		: : "r" (ip));
}

#endif /* _A386_I386_A386_REG_H */
