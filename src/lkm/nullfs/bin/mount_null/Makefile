#	$RuOBSD$

PROG=	mount_null
SRCS=	mount_null.c getmntopts.c
MAN=	mount_null.8

MOUNT=	${.CURDIR}/../mount
CFLAGS+= -I${.CURDIR}/../../include -I /usr/src/sbin/mount
.PATH:		/usr/src/sbin/mount

.include <bsd.prog.mk>
