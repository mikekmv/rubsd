PROG=	pgwrap
SRCS=	pgwrap.c
MAN=

COPTS=	-Wall -Werror -Wtraditional -ansi -pedantic

BINDIR?=/usr/local/bin

OPSYS!=	uname

.if ${OPSYS} == FreeBSD
CFLAGS+=-D__dead=__dead2
LDADD=	-lutil
DPADD=	${LIBUTIL}
.endif

.include <bsd.prog.mk>
