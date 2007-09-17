# $RuOBSD: Makefile,v 1.2 2005/10/28 16:22:59 form Exp $

LKM=	hproc
MAN=

CFLAGS+=-Wall -Werror -Wmissing-prototypes
CPPFLAGS+=-DMULTIPROCESSOR

.if ${MACHINE_ARCH} == amd64
CFLAGS+=-mcmodel=kernel
.endif

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}

.include <bsd.lkm.mk>
