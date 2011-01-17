#	$RuOBSD: Makefile,v 1.4 2011/01/17 06:01:28 dinar Exp $

LKM=	nullfs
SRCS=	null_vfsops.c null_lkm.c null_subr.c null_vnops.c
CFLAGS+=-I${.CURDIR}/../include

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -fno-builtin

MAN=

.include <bsd.lkm.mk>