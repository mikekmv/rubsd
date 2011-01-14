#	$RuOBSD: Makefile,v 1.1.1.1 2011/01/14 08:38:57 dinar Exp $

LKM=	nullfs
SRCS=	null_vfsops.c null_lkm.c null_subr.c null_vnops.c
CFLAGS+=-I${.CURDIR}/../include -g

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes

CFLAGS+=-I${.CURDIR}/../../include -fno-builtin

MAN=

.include <bsd.lkm.mk>