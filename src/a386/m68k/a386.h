/**********************************************************************
m68k/a386.h

Copyright (C) 1999 Lars Brinkhoff.
**********************************************************************/

typedef int a386_word_t;
typedef a386_word_t a386_result_t;
typedef a386_word_t a386_syscall_arg_t;

static inline void
a386_set_kernel_mode (void)
{
  asm ("ori.w #$2000,sr");
}

static inline void
a386_set_user_mode (void)
{
  asm ("andi.w #$dfff,sr");
}

static inline int
a386_in_kernel_mode (void)
{
  int d0;
  asm ("moveq #0,%0; move.w sr,%0; and.w #$2000,%0" : "=d" (d0));
  return d0;
}

static inline int
a386_in_user_mode (void)
{
  return !a386_in_kernel_mode ();
}

static inline void
a386_enable_trace_mode (void)
{
  asm ("ori.w #$8000,sr");
}

static inline void
a386_disable_trace_mode (void)
{
  asm ("andi.w #$7fff,sr");
}

static inline int
a386_in_trace_mode (void)
{
  int d0;
  asm ("move.w sr,%0; and.w #$8000,%0" : "=d" (d0));
  return d0;
}

static inline void
a386_enable_interrupts (void)
{
  asm ("andi.w $f8ff,sr");
}

static inline void
a386_disable_interrupts (void)
{
  asm ("ori.w $0700,d0");
}

static inline int
a386_interrupts_enabled (void)
{
  int d0;
  asm ("move.w sr,d0; and.w #$0700,d0" : "=d" (d0));
  return d0;
}

static inline void
a386_set_vectors (struct a386_vectors *v)
{
  asm ("move.l %0,vbr" : : "d" (v));
}

static inline struct a386_vectors *
a386_get_vectors (void)
{
  int d0;
  asm ("move.l vbr,%0" : "=d" (d0));
  return (struct a386_vectors *)d0;
}

static inline void
a386_set_syscall_arg0 (a386_syscall_arg_t arg)
{
  asm ("move.l %0,d0" : : "m" (arg));
}

static inline void
a386_set_syscall_arg1 (a386_syscall_arg_t arg)
{
  asm ("move.l %0,d1" : : "m" (arg));
}

static inline a386_syscall_result_t
a386_get_syscall_result (void)
{
  a386_syscall_result_t result;
  asm ("move.l d0,%0" : "m" (result));
}

static inline a386_result_t
a386_trap (int n)
{
  a386_retult_t result;
  asm volatile ("movel %0, d0;"
		"int $0x80;"
		"movel d0, %0"
		: "=m" (n)
		: "0" (result));
  return result;
}
