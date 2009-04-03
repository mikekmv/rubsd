# $Bee$
NOMAN=
PROG=	beelogidx
SRCS=	logidx.c 
BINDIR=/usr/local/bin
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lm -lbeedb -lbee -lz 
LDFLAGS+=-L/usr/local/lib
