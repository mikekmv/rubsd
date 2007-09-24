# $RuOBSD: Makefile,v 1.7 2007/09/23 19:49:12 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -g
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee
LDFLAGS+=-L/usr/local/lib
