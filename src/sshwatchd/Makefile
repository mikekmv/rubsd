# $RuOBSD: Makefile,v 1.1.1.1 2006/03/30 12:56:32 form Exp $

PROG=	sshwatchd
MAN=

CFLAGS+=-Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-pedantic

PREFIX?=/usr/local
BINDIR?=${PREFIX}/sbin

LDSTATIC?=-static

.include <bsd.prog.mk>
