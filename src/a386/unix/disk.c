/**********************************************************************
unix/disk.c

Copyright (C) 1999-2000 Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

This file implements a simple disk.

EXTERNAL INTERFACE

a386_disk_init()
a386_disk_seek()
a386_disk_read()
a386_disk_write()

TODO

Simulate I/O delays and generate an interrupt when I/O is complete.
**********************************************************************/

#include <fcntl.h>
#include <unistd.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#define DEBUG_ON a386_debug_disk
#define DEBUG_NAME "disk"
#define DEBUG_DEFAULT 1
#include "debug_stub.h"

struct
{
  long position;
  int fd;
} disk;

int
a386_disk_init (void)
{
  disk.fd = unix_open ("./root_fs", O_RDWR);
  if (disk.fd == -1)
    return -1;
  return 0;
}

int
a386_disk_seek (long sector)
{
  disk.position = sector;
  debug ("seek to %08x", sector);
  unix_lseek (disk.fd, sector << 9, SEEK_SET);
  return 0;
}

long
a386_disk_read (void *buffer, long sectors)
{
  int n;

  debug ("read %d sectors from %08x", sectors, disk.position);
  n = unix_read (disk.fd, buffer, sectors << 9);
  if (n == -1)
    return -1;

  disk.position += sectors;
  return n >> 9;
}

long
a386_disk_write (void *buffer, long sectors)
{
  int n;

  debug ("write %d sectors to %08x", sectors, disk.position);
  n = unix_write (disk.fd, buffer, sectors << 9);
  if (n == -1)
    return -1;

  disk.position += sectors;
  return n >> 9;
}
