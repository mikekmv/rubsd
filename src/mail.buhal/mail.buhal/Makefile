# $RuOBSD: Makefile,v 1.3 2007/12/25 17:22:42 form Exp $

PROG=	mail.buhal
SRCS=	mail.buhal.c
MAN=	mail.buhal.8

CPPFLAGS+=-I${.CURDIR}/../lib

LDSTATIC?=-static

.if exists(${.CURDIR}/../lib/obj/libmaildir.a)
DPADD=	${.CURDIR}/../lib/obj/libmaildir.a
LDADD=	-L${.CURDIR}/../lib/obj -lmaildir
.else
DPADD=	${.CURDIR}/../lib/libmaildir.a
LDADD=	-L${.CURDIR}/../lib -lmaildir
.endif

.include <bsd.prog.mk>
