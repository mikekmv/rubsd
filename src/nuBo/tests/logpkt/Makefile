#	$RuOBSD$

PROG=	logpkt
SRCS=	logpkt.c printers.c
MAN=

COPTS=	-Wall -Werror -ansi -pedantic

LDADD=	-lpcap
DPADD=	${LIBPCAP}

.include <bsd.prog.mk>
