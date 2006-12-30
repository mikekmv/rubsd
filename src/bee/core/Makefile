# $RuOBSD: Makefile,v 1.3 2004/05/02 21:55:53 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c timer.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee
LDFLAGS+=-L/usr/local/lib
