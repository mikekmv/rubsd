# $RuOBSD: Makefile.inc,v 1.1.1.1 2005/03/28 13:56:54 form Exp $

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-fno-builtin-log -fno-builtin-printf

.if ${MACHINE_ARCH} == amd64
CFLAGS+=-mcmodel=kernel
.endif

CPPFLAGS+=-I${.CURDIR}/../../include

MAN?=

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}
