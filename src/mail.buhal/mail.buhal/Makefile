# $RuOBSD$

PROG=	mail.buhal
SRCS=	mail.buhal.c
MAN=	mail.buhal.8

CFLAGS+=-I${.CURDIR}/../lib

COPTS=	-Wall -Werror -Wtraditional -ansi -pedantic

LDADD=	-L${.CURDIR}/../lib/obj -L${.CURDIR}/../lib -lmaildir

.include <bsd.prog.mk>
