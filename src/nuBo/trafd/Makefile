#	$RuOBSD: Makefile,v 1.1.1.1 2003/05/15 09:46:51 grange Exp $

PROG=	trafd
SRCS=	collect.c datalinks.c trafd.c
MAN=	trafd.8

COPTS=	-Wall -Werror

LDADD=	-lpcap
DPADD=	${LIBPCAP}

.include <bsd.prog.mk>
