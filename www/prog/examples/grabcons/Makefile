# $RuOBSD: Makefile,v 1.1 2004/11/01 05:00:44 form Exp $

PROG=	grabcons
MAN=

CFLAGS+=-W -Wall -Werror -Winline -Wshadow -Wcast-qual -Wpointer-arith
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-pedantic

DPADD=	${LIBUTIL}
LDADD=	-lutil

.include <bsd.prog.mk>
