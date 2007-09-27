# $Bee$
NOMAN=
PROG=	ipchk
SRCS=	ipchk.c 
BINDIR=/usr/local/bin
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbee
LDFLAGS+=-L/usr/local/lib
