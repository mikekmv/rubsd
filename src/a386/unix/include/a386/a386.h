/**********************************************************************
a386.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file defines the interface to an abstract Intel 386 machine
running in protected mode.

CPU INTERFACE

a386_init: initialize a386.
a386_version: return a386 version string.
a386_cpu_name: return string describing cpu type.
a386_load: load an ELF executable.
a386_(enable|in)_(kernel|user)_mode: manipulate and query the privilege mode.
a386_(enable|disable)_trace_mode: manipulate and query trace mode.
a386_trap: calls the operating system.
a386_halt: halt processor.
a386_cpu_kzh: return cpu speed in kHz.
a386_(set|get)_usp: manipulate user stack pointer.
a386_(set|get)_arg: manipulate syscall arguments.
a386_panic: print message and abort.

MMU INTERFACE

a386_mmu_name: return string describing mmu type.
a386_((enable|disable)_paging|paging_enabled): manipulate and query
  paging mode.
a386_(get|set)_page_directory
a386_(get|put)_user: read/write user space memory.
a386_virtual_to_physical: translate a virtual address.

TLB INTERFACE

a386_flush_tlb(|_one|_range): flush TLB.

MEMORY INTERFACE

A386_MEMORY_BASE: physical memory base address.
a386_memory_size: physical memory size.

INTERRUPT INTERFACE

a386_interrupt_wait: wait for interrupt.
a386_interrupts_(enable|disable): enable/disable interrupts.
a386_(set|get)_vectors: manipulate vector base register.

DEVICE INTERFACE

a386_device_read
a386_device_write
**********************************************************************/

#ifndef _UNIX_A386_H
#define _UNIX_A386_H

#include <a386/a386_params.h>
#include <a386/a386_types.h>
#include <a386/a386_mmu.h>
#include <a386/a386_vectors.h>
#include <a386/a386_context.h>
#include <a386/a386_reg.h>

#include <a386/a386_debug.h>

#define A386

/* CPU INTERFACE ******************************************************/

/* Manipulate/query the privilege mode. */

void a386_set_kernel_mode (void);
/*void a386_set_user_mode (void);*/
int a386_in_kernel_mode (void);
int a386_in_user_mode (void);

/* Manipulate/query trace mode. */

void a386_set_trace_mode (void);
void a386_unset_trace_mode (void);
int a386_in_trace_mode (void);

/* Call the operating system. */

extern a386_arg_t a386_get_arg (int n);
extern void a386_set_arg (int n, a386_arg_t arg);
extern a386_word_t a386_syscall_number;
/*extern a386_arg_t a386_syscall_arg[5];*/
/*extern a386_result_t a386_trap (int); moved to a386_set_mode.h */

/* Manipulate context. */

int a386_context_get_kernel_mode (const struct a386_context *context);
void a386_context_set_kernel_mode (struct a386_context *context,
				   int kernel_mode);
a386_address_t a386_context_get_sp (const struct a386_context *context);
void a386_context_set_sp (struct a386_context *context,
			  a386_address_t usp);
a386_address_t a386_context_get_ip (const struct a386_context *context);
void a386_context_set_ip (struct a386_context *context,
			  a386_address_t ip);
a386_word_t a386_context_get_syscall (const struct a386_context *context);
void a386_context_set_syscall (struct a386_context *context,
			       a386_word_t syscall);
a386_result_t a386_context_get_result (const struct a386_context *context);
void a386_context_set_result (struct a386_context *context,
			      a386_result_t result);
a386_arg_t a386_context_get_arg (const struct a386_context *context, int n);
void a386_context_set_arg (const struct a386_context *context, int n,
			   a386_arg_t arg);
void a386_context_set (const struct a386_context *context)
     __attribute__ ((noreturn));

/* Halt processor. */

extern void a386_halt (void);
extern void a386_panic (const char *);

extern int a386_cpu_khz (void);

extern a386_address_t a386_get_usp (void);
extern void a386_set_usp (a386_address_t usp);

extern void a386_init (void);
extern void *a386_load (const char *file);

#include <a386/a386_set_mode.h>

/* MMU INTERFACE ******************************************************/

#define A386_PROT_READ	1
#define A386_PROT_WRITE	2
#define A386_PROT_EXEC	4
#define A386_PROT_USER  8

#define A386_REASON_PROT	0x0001
#define A386_REASON_NOPAGE	0x0002
#define A386_REASON_READ	0x0004
#define A386_REASON_WRITE	0x0008
#define A386_REASON_EXEC	0x0010
#define A386_REASON_USER	0x0020
#define A386_REASON_KERNEL	0x0040

const char *a386_mmu_name (void);
void a386_enable_paging (void);
void a386_disable_paging (void);
int a386_paging_enabled (void);

void a386_set_page_directory (struct a386_page_directory_entry *directory);
struct a386_page_directory_entry *a386_get_page_directory (void);

extern a386_word_t a386_get_user (void *address);
extern void a386_put_user (void *address, a386_word_t data);

/* TLB INTERFACE ******************************************************/

extern void a386_tlb_flush (void);
extern void a386_tlb_flush_one (a386_address_t page_frame);
extern void a386_tlb_flush_range (a386_address_t start, a386_address_t end);

/* INTERRUPT INTERFACE ***********************************************/

extern void a386_interrupt_wait (void);
extern void a386_interrupts_enable (void);
extern void a386_interrupts_disable (void);
extern int a386_interrupts_enabled (void);
extern void a386_set_vectors (struct a386_vectors *v);
extern struct a386_vectors *a386_get_vectors (void);

/* MEMORY INTERFACE ***************************************************/

extern a386_address_t a386_memory_size (void);

/* DEVICE INTERFACE ***************************************************/

enum a386_device_space
{
  A386_DEVICE_IO,
  A386_DEVICE_MEMORY
};

void a386_device_read (enum a386_device_space space, void *address,
		       void *data, int length);
void a386_device_write (enum a386_device_space space, void *address,
			void *data, int length);

#endif /* _UNIX_A386_H */
