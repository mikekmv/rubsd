/**********************************************************************
unix/device.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file implements device operations.

EXTERNAL INTERFACE

A386_DEVICE_IO
A386_DEVICE_MEMORY
a386_device_read ()
a386_device_write ()

INTERNAL INTERFACE

_a386_device_init ()
**********************************************************************/

#include "_a386.h"

#define MAX_SPACES 2
#define MAX_DEVICES 10
#define DEVICE_READ 1
#define DEVICE_WRITE 2

struct a386_device
{
  int free;
  struct
  {
    int free;
    enum a386_device_space type;
    a386_address_t start, end;
  } space[MAX_SPACES];
  void (*read) (enum a386_device_space space, a386_address_t address,
		int length, void *data);
  void (*write) (enum a386_device_space space, a386_address_t address,
		 int length, void *data);
};

static struct a386_device device[MAX_DEVICES];

void
_a386_device_init (void)
{
  int i, j;

  for (i = 0; i < MAX_DEVICES; i++)
    {
      device[i].free = 1;
      for (j = 0; j < MAX_SPACES; j++)
	device[i].space[j].free = 1;
    }
}

static void
device_io (int read_write, enum a386_device_space space,
	   a386_address_t address, void *data, int length)
{
  int i, j;

  for (i = 0; i < MAX_DEVICES; i++)
    {
      if (device[i].free)
	continue;

      for (j = 0; j < MAX_SPACES; j++)
	{
	  if (device[i].space[j].free ||
	      device[i].space[j].type != space ||
	      address < device[i].space[j].start ||
	      address >= device[i].space[j].end)
	    continue;

	  if (read_write == DEVICE_READ)
	    device[i].read (space, address, length, data);
	  else /* read_write == DEVICE_WRITE */
	    device[i].write (space, address, length, data);
	}
    }
}

void
a386_device_read (enum a386_device_space space, void *address,
		  void *data, int length)
{
  device_io (DEVICE_READ, space, (a386_address_t)address, data, length);
}

void
a386_device_write (enum a386_device_space space, void *address,
		   void *data, int length)
{
  device_io (DEVICE_WRITE, space, (a386_address_t)address, data, length);
}
