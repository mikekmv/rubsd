/**********************************************************************
unix/tlb.c

Copyright (C) 1999-2000 Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

Translation look-aside buffer implementation.

EXTERNAL INTERFACE

a386_tlb_flush()		Flush all TLB entries.
a386_tlb_flush_one()		Flush one TLB entry.
a386_tlb_flush_range()		Flush all TLB entries within a range.

INTERNAL INTERFACE

_a386_tlb_init()		Initialize TLB.
_a386_tlb_add()			Set random TLB entry.
_a386_tlb_get_virtual()		Get TLB entry virtual page frame.
_a386_tlb_get_physical()	Get TLB entry physical page frame.

INIFINITE TLB

The TLB interface supports any number of TLB entries.  In fact, there
can be "infinitely" many entries.  There is only an operation to add
entries to the TLB.  The TLB itself must take care of reusing old entries
if the TLB table is full.  But in fact the TLB may not want to throw
away old entries, ever.

GETTING A FREE TLB ENTRY

This TLB uses a list of free entries to quickly get an unused entry.
The index of the list head is in the next_free_tlb_entry variable.
Each free entry has a next_free field pointing to the next free entry
in the list.  The list is terminated by a next_free field of -1.

When all TLB entries are occupied next_free_tlb_entry is -1, and
_a386_tlb_add() will pick a (pseudo-)random entry which will be
invalidated before it's recycled.

TODO

* Optimize TLB flushing: call munmap once instead of once per TLB entry.

* Implement A386_PROT_USER.

**********************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#define TLB_ENTRIES 64

#define PAGE_FRAME(a) ((a) & ~_A386_PAGE_MASK)
#define PAGE_OFFSET(a) ((a) & _A386_PAGE_MASK)

#define DEBUG_ON a386_debug_tlb
#define DEBUG_NAME "tlb"
#define DEBUG_DEFAULT 1
#include "debug_stub.h"

struct {
  a386_address_t virtual;
  a386_address_t physical;
  int invalid : 1;
  int dirty : 1;
  int locked : 1;
  int global : 1;
  int protection;
  int next_free;
} tlb[TLB_ENTRIES];

int next_free_tlb_entry;

static void
build_free_list (void)
{
  int i;

  next_free_tlb_entry = -1;
  for (i = 0; i < TLB_ENTRIES; i++)
    {
      if (tlb[i].invalid)
	{
	  tlb[i].next_free = next_free_tlb_entry;
	  next_free_tlb_entry = i;
	}
    }
}

void
_a386_tlb_init (void)
{
  int i;

  for (i = 0; i < TLB_ENTRIES; i++)
    {
      tlb[i].invalid = 1;
      tlb[i].locked = 0;
      tlb[i].global = 0;
    }

  build_free_list ();
}

static void
tlb_invalidate (int index)
{
  int err;

  if (tlb[index].invalid)
    return;

  debug ("unmap %05x from %05x (%c%c%c%c)", tlb[index].virtual >> 12, tlb[index].physical >> 12, tlb[index].protection & A386_PROT_READ ? 'r' : '-', tlb[index].protection & A386_PROT_WRITE ? 'w' : '-', tlb[index].protection & A386_PROT_EXEC ? 'x' : '-', tlb[index].protection & A386_PROT_USER ? 'u' : 'k');
  err = unix_munmap ((void *)tlb[index].virtual,
		     (size_t)_A386_PAGE_SIZE);
  if (err < 0)
    {
      log_error ("munmap error: %d", unix_errno);
      a386_panic ("munmap error in tlb_invalidate()");
    }

  tlb[index].virtual = (a386_address_t)-1;
  tlb[index].physical = (a386_address_t)-1;
  tlb[index].next_free = next_free_tlb_entry;
  next_free_tlb_entry = index;
  tlb[index].invalid = 1;
  tlb[index].locked = 0;
  tlb[index].global = 0;
}

int
_a386_tlb_get_virtual (a386_address_t virtual,
		       a386_address_t *physical,
		       int *protection)
{
  int i;

  virtual = PAGE_FRAME (virtual);

  for (i = 0; i < TLB_ENTRIES; i++)
    {
      if (!tlb[i].invalid &&
	  tlb[i].virtual == virtual)
	{
	  if (physical)
	    *physical = tlb[i].physical;
	  if (protection)
	    *protection = tlb[i].protection;
	  return 1;
	}
    }

  return 0;
}

int
_a386_tlb_get_physical (a386_address_t physical,
			a386_address_t *virtual,
			int *protection)
{
  int i;

  physical = PAGE_FRAME (physical);

  for (i = 0; i < TLB_ENTRIES; i++)
    {
      if (!tlb[i].invalid &&
	  tlb[i].physical == physical)
	{
	  if (virtual)
	    *virtual = tlb[i].virtual;
	  if (protection)
	    *protection = tlb[i].protection;
	  return 1;
	}
    }

  return 0;
}

static void
tlb_set (int index,
	 a386_address_t virtual,
	 a386_address_t physical,
	 int protection)
{
  int unix_protection = 0;
  int err;

  if (protection & A386_PROT_READ)
    unix_protection |= PROT_READ;
  if (protection & A386_PROT_WRITE)
    unix_protection |= PROT_WRITE;
  if (protection & A386_PROT_EXEC)
    unix_protection |= PROT_EXEC;

  tlb_invalidate (index);

  debug ("map %05x to %05x (%c%c%c%c)",
	 virtual >> 12,
	 physical >> 12,
	 protection & A386_PROT_READ ? 'r' : '-',
	 protection & A386_PROT_WRITE ? 'w' : '-',
	 protection & A386_PROT_EXEC ? 'x' : '-',
	 protection & A386_PROT_USER ? 'u' : 'k');

  tlb[index].virtual = virtual;
  tlb[index].physical = physical;
  tlb[index].protection = protection;
  tlb[index].invalid = 0;
  /*tlb[index].locked = ((virtual >> 28) == 9);*/

  next_free_tlb_entry = tlb[index].next_free;
  tlb[index].next_free = -1;
  if (index != next_free_tlb_entry)
    build_free_list ();

a386_printf ("\n         mmap (%p, %d, %d, MAP_FIXED | MAP_SHARED, %d, %p)",
virtual, _A386_PAGE_SIZE, unix_protection, _a386_memory_fd, physical);
  err = unix_mmap ((void *)virtual,
		   (size_t)_A386_PAGE_SIZE,
		   unix_protection,
		   MAP_FIXED | MAP_SHARED,
		   _a386_memory_fd,
		   physical);
  if (err == -1)
    {
      log_error ("mmap error: %d = 0x%x (err = %d)", unix_errno, unix_errno, err);
      a386_panic ("mmap error");
    }
}

void
_a386_tlb_add (a386_address_t virtual,
	       a386_address_t physical,
	       int protection)
{
  static int random_index = 0;
  int index;

#if 1
  for (index = 0; index < TLB_ENTRIES; index++)
    {
      if (tlb[index].virtual == virtual)
	return;
    }
#endif

  index = next_free_tlb_entry;
  if (index < 0)
    {
      do
	{
	  index = random_index;
	  random_index = (random_index + 1) % TLB_ENTRIES;
	}
      while (tlb[index].locked);
    }
  random_index = (random_index + 1) % TLB_ENTRIES;

  tlb_set (index,
	   virtual,
	   physical,
	   protection);
}

void
a386_tlb_flush (void)
{
  int i;

  debug ("flush all");
  for (i = 0; i < TLB_ENTRIES; i++)
    {
      if (!tlb[i].global)
	tlb_invalidate (i);
    }

  build_free_list ();
}

void
a386_tlb_flush_one (a386_address_t address)
{
  int i;

  debug ("flush %p", address);
  address = PAGE_FRAME (address);
  for (i = 0; i < TLB_ENTRIES; i++)
    {
      if (tlb[i].virtual == address)
	tlb_invalidate (i);
    }
}

void
a386_tlb_flush_range (a386_address_t start,
		      a386_address_t end)
{
  int i;

  debug ("flush %p - %p", start, end);
  start = PAGE_FRAME (start);
  end = PAGE_FRAME (end);
  for (i = 0; i < TLB_ENTRIES; i++)
    {
      if (tlb[i].virtual >= start && tlb[i].virtual < end)
	tlb_invalidate (i);
    }
}
