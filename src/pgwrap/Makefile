#	$RuOBSD: Makefile,v 1.2 2001/01/05 09:06:56 form Exp $

PROG=	pgwrap
SRCS=	pgwrap.c

MAN=

CFLAGS+=-Wall -Werror
OPSYS!=	uname -s
.if ${OPSYS:L:Mfreebsd}
LDADD+=	-lutil
.endif

BINDIR=	/usr/local/bin

.include <bsd.prog.mk>
