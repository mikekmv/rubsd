/**********************************************************************
unix/context.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file implements CPU context operations.
**********************************************************************/

#include <signal.h> /* struct sigcontext */
#include <asm/ptrace.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#ifdef i386
#  define REG_IP eip
#  define REG_SP esp
#  define REG_SYSCALL orig_eax
#  define REG_RESULT eax
#endif
#ifdef arm
#  define REG_IP ARM_pc
#  define REG_SP ARM_sp
#  define REG_SYSCALL ARM_ORIG_r0
#  define REG_RESULT ARM_r0
#endif
#ifdef __sparc__
#  define REG_IP pc
#  define REG_SP u_regs[UREG_FP]		/* not sure about this! */
#  define REG_SYSCALL u_regs[UREG_G1]
#  define REG_RESULT u_regs[UREG_I0]		/* not sure about this either! */
#endif

a386_address_t
a386_context_get_ip (const struct a386_context *context)
{
  return ((struct pt_regs *)context->regs)->REG_IP;
}

void
a386_context_set_ip (struct a386_context *context, a386_address_t ip)
{
  ((struct pt_regs *)context->regs)->REG_IP = ip;
}

a386_address_t
a386_context_get_sp (const struct a386_context *context)
{
  return ((struct pt_regs *)context->regs)->REG_SP;
}

void
a386_context_set_sp (struct a386_context *context, a386_address_t sp)
{
  ((struct pt_regs *)context->regs)->REG_SP = sp;
}

a386_word_t
a386_context_get_syscall (const struct a386_context *context)
{
  return ((struct pt_regs *)context->regs)->REG_SYSCALL;
}

void
a386_context_set_syscall (struct a386_context *context, a386_word_t syscall)
{
  ((struct pt_regs *)context->regs)->REG_SYSCALL = syscall;
}

a386_result_t
a386_context_get_result (const struct a386_context *context)
{
  return (a386_result_t)((struct pt_regs *)context->regs)->REG_RESULT;
}

void
a386_context_set_result (struct a386_context *context, a386_result_t result)
{
  ((struct pt_regs *)context->regs)->REG_RESULT = result;
}

a386_address_t
a386_context_get_return (const struct a386_context *context)
{
#ifdef i386
  a386_address_t *sp = (void *)a386_context_get_sp (context);
  return *sp;
#endif
#ifdef arm
  return ((struct pt_regs *)context->regs)->ARM_lr;
#endif
#ifdef __sparc__
  return ((struct pt_regs *)context->regs)->u_regs[UREG_RETPC];
#endif
}

void
a386_context_set_return (struct a386_context *context, a386_address_t ret)
{
#ifdef i386
  a386_address_t *sp = (void *)a386_context_get_sp (context);
  *sp = ret;
#endif
#ifdef arm
  ((struct pt_regs *)context->regs)->ARM_lr = ret;
#endif
#ifdef __sparc__
  ((struct pt_regs *)context->regs)->u_regs[UREG_RETPC] = ret;
#endif
}

int
a386_context_get_kernel_mode (const struct a386_context *context)
{
  return context->kernel_mode;
}

void
a386_context_set_kernel_mode (struct a386_context *context, int kernel_mode)
{
  context->kernel_mode = kernel_mode;
}

int
a386_context_get_interrupts_enabled (const struct a386_context *context)
{
  return context->interrupts_enabled;
}

void
a386_context_set_interrupts_enabled (struct a386_context *context,
				     int interrupts)
{
  context->interrupts_enabled = interrupts;
}

a386_arg_t
a386_context_get_arg (const struct a386_context *context, int n)
{
  return ((a386_arg_t *)context->regs)[n];
}

void
a386_context_set_arg (const struct a386_context *context, int n,
		      a386_arg_t arg)
{
  ((a386_arg_t *)context->regs)[n] = arg;
}

void
_a386_context_get_a386 (struct a386_context *context, int kernel_mode)
{
  context->kernel_mode = kernel_mode;
  context->interrupts_enabled = _a386_interrupts_enabled;
}

int
_a386_context_set_a386 (struct a386_context *context)
{
  return context->kernel_mode;
}

#if 0
void
a386_context_set (const struct a386_context *context)
{
  volatile long ip, sp, result;

  a386_old_kernel_mode = a386_context_get_kernel_mode (context);
  ip = a386_context_get_ip (context);
  sp = a386_context_get_sp (context);
  a386_set_usp (a386_context_get_sp (context));
  ip = a386_context_get_ip (context);
  result = a386_context_get_result (context);
  _a386_restore_mode ();
  asm volatile
    ("jmp *%0"
     : /* no outputs */
     : "r" (ip), "a" (result));

  /* do this to make gcc understand that we really don't return */
  for (;;)
    ;
}

#else

volatile a386_word_t _a386_tmp_ip;
volatile a386_word_t _a386_tmp_sp;
volatile a386_result_t _a386_tmp_result;

#ifdef i386
void __attribute__ ((noreturn))
a386_context_set (const struct a386_context *context)
{
  _a386_tmp_ip = a386_context_get_ip (context);
  _a386_tmp_result = a386_context_get_result (context);
  _a386_tmp_sp = a386_context_get_sp (context);

  if (!a386_context_get_kernel_mode (context))
    {
      /*_a386_memory_unmap ();*/
      /*_a386_ksp = _a386_tmp_sp; don't do this here */
      a386_set_usp (a386_context_get_sp (context));
      unix_kill (unix_getpid (), 10);
      _a386_in_kernel_mode = 0;
    }

  a386_set_sp ((a386_address_t)&context->regs);
  asm volatile
    ("popl %%ebx;"
     "popl %%ecx;"
     "popl %%edx;"
     "popl %%esi;"
     "popl %%edi;"
     "popl %%ebp;"
     "popl %%eax;"
     "addl $4,%%esp;"	/* xds */
     "addl $4,%%esp;"	/* xes */
     "addl $4,%%esp;"	/* orig_eax */
     "addl $4,%%esp;"	/* eip */
     "addl $4,%%esp;"	/* xcs */
     "popf;"		/* eflags */
     "addl $4,%%esp;"	/* esp */
     "addl $4,%%esp;"	/* xss */
     "addl $4,%%esp;"	/* kernel_mode */
     "addl $4,%%esp;"	/* interrupts_enabled */
     : 
     : 
     : "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "cc");

  asm volatile
    ("movl %0,%%esp;"
     "movl %1,%%eax;"
     "jmp *%2;"
     : /* no outputs */
     : "m" (_a386_tmp_sp),
       "m" (_a386_tmp_result),
       "m" (_a386_tmp_ip));

  for (;;)
    ;
}
#endif /* i386 */

#endif
