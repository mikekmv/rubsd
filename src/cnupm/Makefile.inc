# $RuOBSD$

OPSYS!=	uname

BINDIR=	/usr/local/sbin

.ifndef OPSYS
OPSYS!=	uname
.endif

.if ${OPSYS} == FreeBSD
MANDIR=	/usr/local/man/man
.else
MANDIR=	/usr/local/man/cat
.endif
