CC = gcc
CPPFLAGS = -DA386_DEBUG -I./include
CFLAGS = -O2 -g -Wall -Wno-uninitialized

LIBGCC := `$(CC) --print-libgcc-file-name`

OBJS = cpu.o mmu.o tlb.o memory.o interrupt.o timer.o device.o rtc.o \
	context.o sysdep.o memcpy.o debug.o serial.o disk.o

LOAD_OBJS = load.o elf.o memory.o debug.o

all: all-stuff

ifeq (.depend,$(wildcard .depend))
include .depend
all-stuff: a386.a load
else
all-stuff: dep a386.a load
endif

dep: include/a386/sysdep.h
	gcc $(CPPFLAGS) -M *.c >.depend 2>/dev/null

clean:
	rm -f *.o a386.a load include/a386/sysdep.h

tags:
	etags *.[ch]

a386.a: $(OBJS)
	rm -f a386.a
	ar r a386.a $(OBJS)
	ranlib a386.a

load: $(LOAD_OBJS)
	gcc $(LOAD_OBJS) -o load

include/a386/sysdep.h: sysdep/probe
	sysdep/probe >include/a386/sysdep.h

sysdep/probe: sysdep/probe.o

sysdep/probe.o: sysdep/os/probe.c
	gcc -c sysdep/os/probe.c -o sysdep/probe.o
