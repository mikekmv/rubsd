# $RuOBSD: Makefile,v 1.6 2007/09/21 10:22:52 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -g
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee -lbeecfg -lbeesup
LDFLAGS+=-L/usr/local/lib
