# $RuOBSD$

PROG=	sshwatchd
MAN=

CFLAGS+=-Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-pedantic

PREFIX?=/usr/local
BINDIR?=${PREFIX}/sbin

.include <bsd.prog.mk>
