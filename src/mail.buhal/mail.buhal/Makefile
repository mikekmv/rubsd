# $RuOBSD: Makefile,v 1.2 2005/03/28 09:16:09 form Exp $

PROG=	mail.buhal
SRCS=	mail.buhal.c
MAN=	mail.buhal.8

CPPFLAGS+=-I${.CURDIR}/../lib

.if exists(${.CURDIR}/../lib/obj/libmaildir.a)
DPADD=	${.CURDIR}/../lib/obj/libmaildir.a
LDADD=	-L${.CURDIR}/../lib/obj -lmaildir
.else
DPADD=	${.CURDIR}/../lib/libmaildir.a
LDADD=	-L${.CURDIR}/../lib -lmaildir
.endif

.include <bsd.prog.mk>
