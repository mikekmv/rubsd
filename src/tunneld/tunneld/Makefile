#	$RuOBSD$

PROG=	tunneld
SRCS=	tunneld.c
MAN=

COPTS=	-Wall -Werror -DCRYPTO -DNAT -I${.CURDIR}/../libalias
LDADD=	-lutil -L${.CURDIR}/../libalias -lalias
DPADD=	${LIBUTIL}

.include <bsd.prog.mk>
