# $Bee$
NOMAN=
PROG=	acc_off
SRCS=	acc_off.c 
BINDIR=/usr/local/bin
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbee
LDFLAGS+=-L/usr/local/lib
