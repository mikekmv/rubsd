# $RuOBSD: Makefile,v 1.2 2001/09/12 05:03:21 tm Exp $
NOMAN=
PROG=	beeipf
SRCS=	beeipf.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbee
LDFLAGS+=-L/usr/local/lib
