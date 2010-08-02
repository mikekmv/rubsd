# $RuOBSD: Makefile,v 1.2 2007/12/25 17:09:28 form Exp $

PROG=	sshwatchd
MAN=

CFLAGS+=-Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations

PREFIX?=/usr/local
BINDIR?=${PREFIX}/sbin

LDSTATIC?=-static

.include <bsd.prog.mk>
