/**********************************************************************
a386/disk.h

Copyright (C) 1999, 2000 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file defines the interface to an abstract disk.

a386_disk_init()
a386_disk_seek()
a386_disk_read()
a386_disk_write()
**********************************************************************/

#ifndef _A386_DISK_H
#define _A386_DISK_H

extern int a386_disk_init (void);
extern int a386_disk_seek (long sector);
extern long a386_disk_read (void *buffer, long sectors);
extern long a386_disk_write (void *buffer, long sectors);

#endif /* _A386_DISK_H */
