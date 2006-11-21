# $RuOBSD$

PROG=	eventsrv
MAN=

CFLAGS+=-W -Wall -Werror -Winline -Wshadow -Wcast-qual -Wpointer-arith
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-pedantic

DPADD=	${LIBEVENT}
LDADD=	-levent

.include <bsd.prog.mk>
