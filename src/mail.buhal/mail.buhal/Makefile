# $RuOBSD: Makefile,v 1.1 2002/12/18 03:35:17 form Exp $

PROG=	mail.buhal
SRCS=	mail.buhal.c
MAN=	mail.buhal.8

CFLAGS+=-I${.CURDIR}/../lib

COPTS=	-Wall -Werror -pedantic

LDADD=	-L${.CURDIR}/../lib/obj -L${.CURDIR}/../lib -lmaildir

.include <bsd.prog.mk>
