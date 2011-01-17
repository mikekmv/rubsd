#	$RuOBSD: Makefile,v 1.3 2011/01/17 05:53:44 dinar Exp $

LKM=	nullfs
SRCS=	null_vfsops.c null_lkm.c null_subr.c null_vnops.c
CFLAGS+=-I${.CURDIR}/../include

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes

MAN=

.include <bsd.lkm.mk>