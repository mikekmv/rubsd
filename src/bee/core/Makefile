# $Bee$
NOMAN=
PROG=	bee
SRCS=	core.c command.c links.c res.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee
LDFLAGS+=-L/usr/local/lib
