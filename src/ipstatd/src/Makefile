#	$Id$

PROG=	ipstatd
CC=gcc
MAN=		
#CFLAGS = -O2
CFLAGS = -g -DDEBUG -DDIAGNOSTIC
#CFLAGS = -g -DDEBUG -Wall

all:		ipstatd dumpstat

mydaemon.o:	mydaemon.c
		${CC} ${CFLAGS} -c $> -o $@

challenge.o:	challenge.c
		${CC} ${CFLAGS} -c $> -o $@

ipfil.o:	ipfil.c
		${CC} ${CFLAGS} -c $> -o $@

net.o:		net.c
		${CC} ${CFLAGS} -c $> -o $@

test:		test.c challenge.o
		${CC} ${CFLAGS} $> -o $@

ipstatd:	ipstatd.c challenge.o mydaemon.o net.o ipfil.o
		${CC} ${CFLAGS} $> -o $@

dumpstat:	dumpstat.c
		${CC} ${CFLAGS} $> -o $@

clean:	
		rm -f *.o
		rm -f ipstatd dumpstat
