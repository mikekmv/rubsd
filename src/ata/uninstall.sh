#!/bin/sh
#
#	$RuOBSD$

SRCDIR=/usr/src

rm $SRCDIR/sys/arch/i386/conf/NEWATA

mv $SRCDIR/sys/arch/i386/conf/files.i386.orig \
   $SRCDIR/sys/arch/i386/conf/files.i386

mv $SRCDIR/sys/arch/i386/i386/autoconf.c.orig \
   $SRCDIR/sys/arch/i386/i386/autoconf.c

mv $SRCDIR/sys/arch/i386/i386/conf.c.orig \
   $SRCDIR/sys/arch/i386/i386/conf.c

mv $SRCDIR/sys/dev/ata/ata.c.orig \
   $SRCDIR/sys/dev/ata/ata.c

mv $SRCDIR/sys/dev/ata/atavar.h.orig \
   $SRCDIR/sys/dev/ata/atavar.h

mv $SRCDIR/sys/dev/ata/files.ata.orig \
   $SRCDIR/sys/dev/ata/files.ata

rm $SRCDIR/sys/dev/pci/atapci.c
rm $SRCDIR/sys/dev/pci/atapcivar.h

mv $SRCDIR/sys/dev/pci/files.pci.orig \
   $SRCDIR/sys/dev/pci/files.pci
