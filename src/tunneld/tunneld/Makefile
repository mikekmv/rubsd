#	$RuOBSD: Makefile,v 1.2 2001/11/14 13:20:31 form Exp $

PROG=	tunneld
SRCS=	tunneld.c
MAN=

COPTS=	-Wall -Werror -DCRYPT -DNAT -I${.CURDIR}/../libalias
LDADD=	-lutil -L${.CURDIR}/../libalias -lalias
DPADD=	${LIBUTIL}

BINDIR?=/usr/local/sbin

.include <bsd.prog.mk>
