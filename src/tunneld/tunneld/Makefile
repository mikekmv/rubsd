#	$RuOBSD: Makefile,v 1.1.1.1 2001/11/14 13:14:21 form Exp $

PROG=	tunneld
SRCS=	tunneld.c
MAN=

COPTS=	-Wall -Werror -DCRYPTO -DNAT -I${.CURDIR}/../libalias
LDADD=	-lutil -L${.CURDIR}/../libalias -lalias
DPADD=	${LIBUTIL}

BINDIR?=/usr/local/sbin

.include <bsd.prog.mk>
