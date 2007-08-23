# $RuOBSD: Makefile,v 1.4 2006/12/30 09:24:22 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c timer.c tariffs.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee
LDFLAGS+=-L/usr/local/lib
