/**********************************************************************
m68k/a386_reg.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

a386_get/set_sp()		Get/set stack pointer.
a386_get_ret_result()		Get/set result.
a386_get/set_ip()		Get/set instruction pointer.
**********************************************************************/

extern inline void *
a386_get_sp (void)
{
  register void *sp asm ("%sp");
  return sp;
}

extern inline void
a386_set_sp (void *sp)
{
  asm volatile ("movel %0,%%sp" : : "r" (sp));
}

extern inline a386_result_t
a386_get_result (void)
{
  register a386_result_t result asm ("%d0");
  return result;
}

extern inline void
a386_set_result (a386_result_t result)
{
  asm volatile ("movel %0,%%d0" : : "mr" (result) : "d0");
}

extern inline void
a386_set_ip (void *ip)
{
  asm volatile ("jmp (%0)\n"
		: : "a" (ip));
}
