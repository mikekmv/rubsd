/**********************************************************************
unix/mmu.c

Copyright (C) 1999-2000 Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

This file implements the memory management unit, except for the
translation look-aside buffer, which is implemented in tlb.c.

EXTERNAL INTERFACE

a386_mmu_name			Return string describing the MMU type.
a386_enable_paging()		Enable paging.
a386_disable_paging()		Disable paging.
a386_paging_enabled()		Paging enabled?
a386_set_page_directory()	Write page directory register.
a386_get_page_directory()	Read page directory register.
a386_get_user()			Get a word from user space memory.
a386_put_user()			Put a word into user space memory.
a386_virtual_to_physical()	Translate a virtual address.
a386_mmu_ ... ()

INTERNAL INTERFACE

_a386_mmu_init()		Initialize MMU.
_a386_page_fault_address	Page fault address.
_a386_page_fault_reason		Page fault reason.
_a386_page_fault_handler()	Page fault handler.
_a386_mmu_map_address()		Ensure that a page is mapped.

TODO

* implement user_access bit in page table entries?
**********************************************************************/

/* MMU type */
#define MMU_NAME "i386"

#include <sys/ptrace.h>
#include "_a386.h"

#define DEBUG_ON a386_debug_mmu
#define DEBUG_NAME "mmu"
#define DEBUG_DEFAULT 2
#include "debug_stub.h"

static struct a386_page_directory_entry *page_directory;
static int _a386_paging_enabled = 0;
a386_address_t _a386_page_fault_address;
int _a386_page_fault_reason;

const char *
a386_mmu_name (void)
{
  return MMU_NAME;
}

void
_a386_mmu_init (void)
{
  _a386_tlb_init ();
}

void
a386_enable_paging (void)
{
  _a386_assert_kernel_mode ();
  _a386_paging_enabled = 1;
}

void
a386_disable_paging (void)
{
  _a386_assert_kernel_mode ();
  _a386_paging_enabled = 0;
  a386_tlb_flush ();
}

int
a386_paging_enabled (void)
{
  _a386_assert_kernel_mode_return (int);
  return _a386_paging_enabled;
}

void
a386_set_page_directory (struct a386_page_directory_entry *directory)
{
  _a386_assert_kernel_mode ();
  if (_a386_paging_enabled && page_directory != directory)
    a386_tlb_flush ();
  page_directory = directory;
  debug ("page directory := %p", page_directory);
}

struct a386_page_directory_entry *
a386_get_page_directory (void)
{
  _a386_assert_kernel_mode_return (struct a386_page_directory_entry *);
  return page_directory;
}

a386_word_t
a386_get_user (void *address)
{
#if 0
  long data;
  unix_ptrace (PTRACE_PEEKDATA, child, address, &data);
  return data;
#else
  return *(a386_word_t *)address;
#endif
}

void
a386_put_user (void *address, a386_word_t data)
{
#if 0
  return (a386_word_t)ptrace (PTRACE_POKEDATA, child, address, data);
#else
  *(a386_word_t *)address = data;
#endif
}

static inline int
virtual_to_physical_page (a386_address_t virtual, a386_address_t *physical,
			  int *protection)
{
  struct a386_page_directory_entry *directory_entry;
  struct a386_page_table_entry *table_entry;
  a386_address_t page_table_p;

  if (!_a386_paging_enabled)
    return 0;

  directory_entry = &page_directory[virtual >> 22];
  /* FIXME: check that directory_entry is a valid address */
  if (!directory_entry->present)
    return 0;

  page_table_p = (directory_entry->page_table << 12) + A386_MEMORY_BASE;
  table_entry =
    &((struct a386_page_table_entry *)page_table_p)[(virtual >> 12) & 0x3ff];
  /* FIXME: check that table_entry is a valid address */
  if (!table_entry->present)
    return 0;

  *physical = table_entry->page_frame << 12;
  *protection = A386_PROT_READ | A386_PROT_EXEC;
  if (table_entry->write_access)
    *protection |= A386_PROT_WRITE;
  if (table_entry->user_access)
    *protection |= A386_PROT_USER;

  return 1;
}

/* FIXME: lame error reporting: 0 is a valid physical address */
a386_address_t
a386_virtual_to_physical (a386_address_t virtual)
{
  a386_address_t physical;
  int protection;
  if (!virtual_to_physical_page (virtual, &physical, &protection))
    return 0;
  return physical | (virtual & 0xfff);
}

void
_a386_page_fault_handler (a386_address_t address, int reason,
			  a386_address_t ksp)
{
  a386_address_t physical;
  int protection;
  
#if 1
char reas[16];
if (reason & A386_REASON_PROT) reas[0] = 'p'; else reas[0] = 'n';
if (reason & A386_REASON_WRITE) reas[1] = 'w'; else reas[1] = 'r';
if (reason & A386_REASON_KERNEL) reas[2] = 'k'; else reas[2] = 'u';
reas[4] = 0;
#endif

  if (!_a386_paging_enabled)
    {
      _a386_vectors->segmentation_violation ();
      return;
    }

  _a386_page_fault_address = address;
  _a386_page_fault_reason = reason;

  if (!virtual_to_physical_page (address, &physical, &protection))
    {
      debug ("page fault (%s) at %p handled by kernel", reas, address);
      a386_set_sp (ksp);
      a386_set_ip ((a386_address_t)_a386_vectors->page_fault);
    }

  if ((reason & A386_REASON_WRITE) && !(protection & A386_PROT_WRITE))
    {
      debug ("page fault (%s) at %p handled by kernel", reas, address);
      a386_set_sp (ksp);
      a386_set_ip ((a386_address_t)_a386_vectors->page_fault);
    }

  debug ("page fault (%s) at %p handled by MMU", reas, address);
  _a386_tlb_add (address & ~_A386_PAGE_MASK, physical, protection);
}

void
_a386_mmu_map_address (a386_address_t address)
{
  a386_address_t physical;
  int protection;
  
  if (_a386_tlb_get_virtual (address, 0, 0))
    return;

  if (!virtual_to_physical_page (address, &physical, &protection))
    return;

  _a386_tlb_add (address & ~_A386_PAGE_MASK, physical, protection);
}

#if 0
void
a386_mmu_ ... (void)
{
  a386_address_t a;
  int p;

  if (virtual_to_physical_page (_a386_page_fault_address, &a, &p))
    _a386_tlb_add (_a386_page_fault_address & ~_A386_PAGE_MASK, a, p);
}
#endif
