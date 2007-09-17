# $RuOBSD: Makefile.inc,v 1.2 2005/10/28 16:22:58 form Exp $

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-fno-builtin-log -fno-builtin-printf

.if ${MACHINE_ARCH} == amd64
CFLAGS+=-mcmodel=kernel
.endif

CPPFLAGS+=-I${.CURDIR}/../../include -DMULTIPROCESSOR

MAN?=

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}
