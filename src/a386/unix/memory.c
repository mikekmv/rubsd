/**********************************************************************
unix/memory.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

Physical memory.

EXTERNAL INTERFACE

A386_MEMORY_BASE		Physical memory start address.
a386_memory_size()		Returns the size of the installed memory.
a386_copy_from_user()

INTERNAL INTERFACE

_a386_memory_fd			Physical memory file descriptor.
_a386_memory_init()		Initialize memory.
_a386_memory_map()		Map physical memory.
_a386_memory_unmap()		Unmap physical memory.
**********************************************************************/

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#define DEBUG_ON a386_debug_memory
#define DEBUG_NAME "memory"
#define DEBUG_DEFAULT 1
#include "debug_stub.h"

int _a386_memory_fd = -1;
static size_t memory_size;

a386_address_t
a386_memory_size (void)
{
  return memory_size;
}

void
_a386_memory_map (void)
{
  int err;

  debug ("map");
  err = unix_mmap ((void *)A386_MEMORY_BASE,
		   memory_size,
		   PROT_READ | PROT_WRITE | PROT_EXEC,
		   MAP_FIXED | MAP_SHARED,
		   _a386_memory_fd,
		   0);
  if (err == -1)
    log_error ("map: mmap error: %d", unix_errno);
}

void
_a386_memory_unmap (void)
{
  debug ("unmap");
#if 0
  int err;
  err = unix_munmap ((void *)A386_MEMORY_BASE, memory_size);
  if (err == -1)
    log_error ("unmap: munmap error: %d", unix_errno);
#endif
}

void
_a386_memory_init (void)
{
  if (_a386_memory_fd != -1)
    return;

  debug ("init");

  _a386_memory_fd = unix_open ("memory", O_RDWR);
  if (_a386_memory_fd == -1)
    log_error ("open error: %d", unix_errno);

  memory_size = unix_lseek (_a386_memory_fd, 0, SEEK_END);
  if (memory_size == -1)
    log_error ("init: lseek error: %d", unix_errno);

  memory_size &= -_A386_PAGE_SIZE;
  _a386_memory_map ();
}

void
a386_memory_fixme (void)
{
  debug ("fixme");
  unix_munmap (0, A386_MEMORY_BASE);
}

#ifndef DUAL_SPACE

void
a386_copy_from_user (void *to, void *from, size_t len)
{
}

#else /* DUAL_SPACE */

void
_a386_copy_from_user (void *_to, void *_from, size_t len)
{
  long x;
  long *to = _to;
  long *from = _from;

  while (len > sizeof (long))
    {
      ptrace (PTRACE_PEEKDATA, child, from++, to++);
      len -= 4;
    }

  if (len > 0)
    a386_copy_from_user (to, from, len);
}

static inline void
a386_copy_from_user (void *to, void *from, size_t len)
{
  long x;

#ifdef WORD_BIGENDIAN
  if (len == 1)
    {
      unix_ptrace (PTRACE_PEEKDATA, child, from, &x);
    }
  else if (len == 2)
    {
    }
#ifdef WORD_32
  else if (len == 4)
    {
    }
#else
  else if (len == 4)
    {
    }
  else if (len == 8)
    {
    }
#endif
#else
  if (len == 1)
    {
    }
  else if (len == 2)
    {
    }
  else if (len == 4)
    {
    }
  else if (len == 8)
    {
    }
#endif
  else
    {
      a386_copy_from_user (to, from, len);
    }
}

#endif /* DUAL_SPACE */
