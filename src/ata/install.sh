#!/bin/sh
#
#	$RuOBSD$

SRCDIR=/usr/src

cp sys/arch/i386/conf/NEWATA $SRCDIR/sys/arch/i386/conf

mv $SRCDIR/sys/arch/i386/conf/files.i386 \
   $SRCDIR/sys/arch/i386/conf/files.i386.orig
cp sys/arch/i386/conf/files.i386 $SRCDIR/sys/arch/i386/conf

mv $SRCDIR/sys/arch/i386/i386/autoconf.c \
   $SRCDIR/sys/arch/i386/i386/autoconf.c.orig
cp sys/arch/i386/i386/autoconf.c $SRCDIR/sys/arch/i386/i386

mv $SRCDIR/sys/arch/i386/i386/conf.c \
   $SRCDIR/sys/arch/i386/i386/conf.c.orig
cp sys/arch/i386/i386/conf.c $SRCDIR/sys/arch/i386/i386

mv $SRCDIR/sys/dev/ata/ata.c \
   $SRCDIR/sys/dev/ata/ata.c.orig
cp sys/dev/ata/ata.c $SRCDIR/sys/dev/ata

mv $SRCDIR/sys/dev/ata/atavar.h \
   $SRCDIR/sys/dev/ata/atavar.h.orig
cp sys/dev/ata/atavar.h $SRCDIR/sys/dev/ata

cp sys/dev/ata/atavar.h $SRCDIR/sys/dev/ata

mv $SRCDIR/sys/dev/ata/files.ata \
   $SRCDIR/sys/dev/ata/files.ata.orig
cp sys/dev/ata/files.ata $SRCDIR/sys/dev/ata

cp sys/dev/pci/atapci.c $SRCDIR/sys/dev/pci
cp sys/dev/pci/atapcivar.h $SRCDIR/sys/dev/pci

mv $SRCDIR/sys/dev/pci/files.pci \
   $SRCDIR/sys/dev/pci/files.pci.orig
cp sys/dev/pci/files.pci $SRCDIR/sys/dev/pci
