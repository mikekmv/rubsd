# $Bee$
NOMAN=
PROG=	getacc2
SRCS=	getacc2.c 
BINDIR=/usr/local/bin
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbee
LDFLAGS+=-L/usr/local/lib
