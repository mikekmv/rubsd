#	$Id$

PROG=	ipstatd
CC=gcc
MAN=		
CFLAGS = -g -DDEBUG

all:		ipstatd dumpstat

daemon.o:	daemon.c
		${CC} ${CFLAGS} -c $> -o $@

challenge.o:	challenge.c
		${CC} ${CFLAGS} -c $> -o $@

test:		test.c challenge.o
		${CC} ${CFLAGS} $> -o $@

ipstatd:	ipstatd.c challenge.o daemon.o
		${CC} ${CFLAGS} $> -o $@

dumpstat:	dumpstat.c
		${CC} ${CFLAGS} $> -o $@

clean:	
		rm -f *.o
