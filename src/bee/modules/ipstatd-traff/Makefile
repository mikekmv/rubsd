# $RuOBSD: Makefile,v 1.1 2002/04/01 02:36:42 shadow Exp $
NOMAN=
PROG=	beecisco
SRCS=	beetraff.c res.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lbee 
LDFLAGS+=-L/usr/local/lib
