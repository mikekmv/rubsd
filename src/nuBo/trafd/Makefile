#	$RuOBSD$

PROG=	trafd
SRCS=	collect.c datalinks.c trafd.c
MAN=

COPTS=	-Wall -Werror

LDADD=	-lpcap
DPADD=	${LIBPCAP}

.include <bsd.prog.mk>
