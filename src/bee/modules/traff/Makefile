# $RuOBSD: Makefile,v 1.2 2004/05/08 15:35:55 shadow Exp $
NOMAN=
PROG=	beetraff
SRCS=	beetraff.c res.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lbee 
LDFLAGS+=-L/usr/local/lib
