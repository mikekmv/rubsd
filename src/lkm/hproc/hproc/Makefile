# $RuOBSD: Makefile,v 1.1.1.1 2004/04/05 10:25:19 form Exp $

LKM=	hproc
MAN=

CFLAGS+=-Wall -Werror -Wmissing-prototypes

.if ${MACHINE_ARCH} == amd64
CFLAGS+=-mcmodel=kernel
.endif

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}

.include <bsd.lkm.mk>
