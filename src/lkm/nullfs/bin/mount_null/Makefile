#	$RuOBSD: Makefile,v 1.1.1.1 2011/01/14 08:38:57 dinar Exp $

PROG=	mount_null
SRCS=	mount_null.c getmntopts.c
MAN=	mount_null.8

MOUNT=	${.CURDIR}/../mount
CFLAGS+= -I${.CURDIR}/../../include -I /usr/src/sbin/mount
.PATH:		/usr/src/sbin/mount

PREFIX?=/usr/local
BINDIR=${PREFIX}/sbin

.include <bsd.prog.mk>
