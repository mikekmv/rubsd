/**********************************************************************
unix/elf.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

Load an ELF executable.

EXTERNAL INTERFACE

a386_load			Load ELF executable and return entry point.

**********************************************************************/

#include <elf.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

static void
memclear (void *addr_, int len)
{
  char *addr = addr_;
  int i;

  for (i = 0; i < len; i++)
    *addr++ = 0;
}

static int
read_all (int fd, void *addr_, int len)
{
  char *addr = addr_;
  int N = len;

  while (len > 0)
    {
      int n;

      n = read (fd, addr, len);
      if (n <= 0)
	{
	  perror ("read");
	  return -1;
	}

      addr += n;
      len -= n;
    }

  return N;
}

static void *
load (int fd)
{
  Elf32_Ehdr header;
  Elf32_Shdr sheader;
  void *entry;
  int i;

  if (read_all (fd, &header, sizeof header) < 0)
    return 0;
    
  if (header.e_ident[EI_MAG0] != ELFMAG0 ||
      header.e_ident[EI_MAG1] != ELFMAG1 ||
      header.e_ident[EI_MAG2] != ELFMAG2 ||
      header.e_ident[EI_MAG3] != ELFMAG3)
    {
      fprintf (stderr, "load: no magic number\n");
      errno = ENOEXEC;
      return 0;
    }

  entry = (void *)header.e_entry;

  /* load ELF executable sections */
  for (i = 0; i < header.e_shnum; i++)
    {
      if (i == SHN_UNDEF)
	continue;

      /* read section header */
      if (lseek (fd, header.e_shoff + i * header.e_shentsize, SEEK_SET) < 0)
	return 0;
      if (read_all (fd, &sheader, sizeof sheader) < 0)
	return 0;

      if (!(sheader.sh_flags & SHF_ALLOC))
	continue;
      /* other flags: SHF_WRITE, SHF_EXECINSTR */

      switch (sheader.sh_type)
	{
	case SHT_PROGBITS:
	  /* text or data section, load in memory */
	  if (lseek (fd, sheader.sh_offset, SEEK_SET) < 0)
	    return 0;
	  if (read_all (fd, (void *)sheader.sh_addr, sheader.sh_size) < 0)
	    return 0;
	  break;
	case SHT_NOBITS:
	  /* bss section, clear memory */
	  memclear ((void *)sheader.sh_addr, sheader.sh_size);
	  break;
	default:
	  /* do nothing */
	}
    }

  return entry;
}

static void *
load_elf (const char *file)
{
  void *result;
  int fd;

  fd = unix_open (file, O_RDONLY);
  if (fd < 0)
    {
      perror ("load_elf");
      return 0;
    }

  result = load (fd);
  close (fd);
  return result;
}

void *
a386_load (const char *file)
{
  _a386_memory_init (); /* map physical memory */
  return load_elf (file);
}
