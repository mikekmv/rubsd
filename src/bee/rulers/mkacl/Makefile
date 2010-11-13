# $RuOBSD: Makefile,v 1.3 2002/06/02 13:20:28 shadow Exp $
NOMAN=
PROG=	beemkacl
SRCS=	mkacl.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../../include
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeedb -lbee
LDFLAGS+=-L/usr/local/lib
