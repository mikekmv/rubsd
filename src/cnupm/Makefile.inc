# $RuOBSD: Makefile.inc,v 1.3 2004/04/19 12:53:40 form Exp $

COPTS+=	-Wall -Werror -Wmissing-prototypes -Wstrict-prototypes -Winline
COPTS+=	-Wshadow -Wpointer-arith -Wmissing-declarations -Wno-long-long
COPTS+=	-Wno-format
COPTS+=	-I${.CURDIR}/../include

BINDIR?=/usr/local/sbin

.if ${OS} == FreeBSD
MANDIR?=/usr/local/man/man
.endif

.if ${OS} == NetBSD
MANDIR?=/usr/local/man
.endif

.if ${OS} == OpenBSD
MANDIR?=/usr/local/man/cat
.endif

