# $RuOBSD: Makefile,v 1.5 2007/08/23 08:43:46 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c timer.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -g
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee -lbeecfg -lbeesup
LDFLAGS+=-L/usr/local/lib
