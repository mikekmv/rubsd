#ifndef _SPARC_A386_REG_H_
#define _SPARC_A386_REG_H_


static inline a386_address_t
a386_get_sp (void)
{
  register a386_address_t sp asm ("%sp");
  return sp;
}

static inline void
a386_set_sp (a386_address_t sp)
{
  asm volatile ("mov %0,%%sp" : : "r" (sp));
}

static inline a386_result_t
a386_get_result (void)
{
  register a386_address_t result asm ("%o0");
  return result;
}

static inline void
a386_set_result (a386_result_t result)
{
  asm volatile ("mov %0,%%o0" : : "r" (result));
}

static inline void
a386_set_ip (a386_address_t ip)
{
  /* FIXME: this is probably not right */
  asm volatile ("jmp %0" : : "r" (ip));
}

#endif
