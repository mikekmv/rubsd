# $RuOBSD: Makefile.inc,v 1.3 2007/09/17 13:53:56 form Exp $

.ifndef CFLAGS_OK
CFLAGS_OK=
CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-fno-builtin-log -fno-builtin-printf

.if ${MACHINE_ARCH} == amd64
CFLAGS+=-mcmodel=kernel -mno-red-zone -fno-strict-aliasing -mno-sse2 -mno-sse
CFLAGS+=-mno-3dnow -mno-mmx -msoft-float -fno-omit-frame-pointer
.endif

CPPFLAGS+=-I${.CURDIR}/../../include -DMULTIPROCESSOR
.endif

MAN?=

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}
