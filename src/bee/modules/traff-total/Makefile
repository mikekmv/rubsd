# $RuOBSD: Makefile,v 1.1 2007/09/15 15:55:20 shadow Exp $
NOMAN=
PROG=	beetraff-tot
SRCS=	beetraff.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lbee
LDFLAGS+=-L/usr/local/lib
