# $Bee$
NOMAN=
PROG=	beeipf
SRCS=	beeipf.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
#LDADD+= -lbeeipc -lbeedb -lz
LDFLAGS+=-L/usr/local/lib
