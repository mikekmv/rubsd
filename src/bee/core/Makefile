# $RuOBSD: Makefile,v 1.2 2001/09/11 03:14:23 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee
LDFLAGS+=-L/usr/local/lib
