/**********************************************************************
unix/_a386.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file contains internal interfaces.  Do not use them outside of a386.
**********************************************************************/

#ifndef _UNIX__A386_H
#define _UNIX__A386_H

#include <a386/a386.h>

/* CPU INTERFACE ******************************************************/

#define _a386_assert_kernel_mode_return(T) \
if (!_a386_in_kernel_mode) \
  { \
    a386_result_t result; \
    _a386_set_kernel_mode (); \
    result = _a386_vectors->privilege_violation (__FUNCTION__); \
    _a386_restore_mode (); \
    return (T)result; \
  }

#include <a386/a386_set_mode.h>

/* MMU INTERFACE ******************************************************/

extern void _a386_mmu_init (void);
extern void _a386_mmu_map_address (a386_address_t address);
extern void _a386_page_fault_handler (a386_address_t, int, a386_address_t);

/* TLB INTERFACE ******************************************************/

extern int _a386_physical_memory_fd;

void _a386_tlb_init (void);
void _a386_tlb_add (a386_address_t page_frame_virt,
		    a386_address_t page_frame_phys,
		    int protection);
int _a386_tlb_get_virtual (a386_address_t virtual,
			   a386_address_t *physical,
			   int *protection);
int _a386_tlb_get_physical (a386_address_t physical,
			    a386_address_t *virtual,
			    int *protection);

/* MEMORY INTERFACE ***************************************************/

extern int _a386_memory_fd;
extern void _a386_memory_init (void);
extern void _a386_memory_map (void);
extern void _a386_memory_unmap (void);

/* INTERRUPT INTERFACE ************************************************/

extern int  _a386_interrupts_enabled;
extern void _a386_interrupt_init (void);
extern void _a386_device_interrupt (int number);

/* DEVICE INTERFACE ***************************************************/

extern void _a386_device_init (void);

/* CONTEXTS ***********************************************************/

extern void _a386_context_get (struct a386_context *, int);
extern int  _a386_context_set (struct a386_context *);

/* OPERATING SYSTEM AND MACHINE DEPENDANT *****************************/

extern long _a386_cancel_syscall (int pid);
extern void _a386_call_function (int child, void *function);
extern void _a386_jump_to_address (int child, void *address);
void _a386_get_syscall_arguments (int child, a386_arg_t *);
void _a386_terminal_save (int fd, void *x);
void _a386_terminal_raw (int fd);
void _a386_terminal_restore (int fd, void *x);

/* SERIAL PORT *********************************************************/

extern void _a386_serial_poll (void);

#endif /* _UNIX__A386_H */
