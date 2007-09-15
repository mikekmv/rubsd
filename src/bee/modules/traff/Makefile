# $RuOBSD: Makefile,v 1.4 2005/07/30 22:43:13 shadow Exp $
NOMAN=
PROG=	beetraff
SRCS=	beetraff.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lbee 
LDFLAGS+=-L/usr/local/lib
