/**********************************************************************
i386/a386.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.
**********************************************************************/

#include "a386_reg.h"

typedef int a386_word_t;
typedef a386_word_t a386_result_t;
typedef a386_word_t a386_syscall_arg_t;

static inline void
a386_set_kernel_mode (void)
{
}

static inline void
a386_set_user_mode (void)
{
}

static inline int
a386_in_kernel_mode (void)
{
}

static inline int
a386_in_user_mode (void)
{
  return !a386_in_kernel_mode ();
}

static inline void
a386_enable_trace_mode (void)
{
}

static inline void
a386_disable_trace_mode (void)
{
}

static inline int
a386_in_trace_mode (void)
{
}

static inline void
a386_enable_interrupts (void)
{
  asm ("sti");
}

static inline void
a386_disable_interrupts (void)
{
  asm ("cli");
}

static inline int
a386_interrupts_enabled (void)
{
  /* FIXME: something with eflags */
}

static inline void
a386_set_vectors (struct a386_vectors *v)
{
}

static inline struct a386_vectors *
a386_get_vectors (void)
{
}

static inline a386_result_t
a386_trap (int n)
{
  a386_result_t result;
  asm volatile ("int $80" : "=a" (result) : "0" (n));
  return result;
}

extern void a386_init (void);

void
a386_enable_paging (void)
{
}

void
a386_disable_paging (void)
{
}

int
a386_paging_enabled (void)
{
}

void
a386_set_page_directory (struct a386_page_directry_entry *directory)
{
  asm volatile ("movl %0, %%cr3" : "=r" (directory));
}

struct a386_page_directry_entry *
a386_get_page_directory (void)
{
  struct a386_page_directory_entry *directory;

  asm volatile ("movl %%cr3, %0\n" : "=r" (directory));
  return directory;
}
