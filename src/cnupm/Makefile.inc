# $RuOBSD: Makefile.inc,v 1.1.1.1 2003/10/07 07:25:09 form Exp $

OPSYS!=	uname

BINDIR=	/usr/local/sbin

.ifndef OPSYS
OPSYS!=	uname
.endif

.if ${OPSYS} == FreeBSD
MANDIR=	/usr/local/man/man
.endif

.if ${OPSYS} == NetBSD
MANDIR=	/usr/local/man
.endif

.if ${OPSYS} == OpenBSD
MANDIR=	/usr/local/man/cat
.endif
