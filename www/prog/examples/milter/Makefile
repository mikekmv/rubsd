# $RuOBSD$

PROG=	milter
MAN=

CFLAGS+=-W -Wall -Werror -Winline -Wshadow -Wcast-qual -Wpointer-arith
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-pedantic -Wno-unused-parameter

DPADD=	${LIBMILTER}
LDADD=	-lmilter

LDFLAGS+=-pthread

.include <bsd.prog.mk>
