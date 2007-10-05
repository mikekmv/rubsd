# $RuOBSD$

LKM=	rucd
MAN=

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-fno-builtin-log -fno-builtin-printf

.if ${MACHINE_ARCH} == amd64
CFLAGS+=-mcmodel=kernel -mno-red-zone -fno-strict-aliasing -mno-sse2 -mno-sse
CFLAGS+=-mno-3dnow -mno-mmx -msoft-float -fno-omit-frame-pointer
.endif

.if defined(NO_PROPOLICE) && ${NO_PROPOLICE:U} == YES
CFLAGS+=-fno-stack-protector
.endif

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}

.include <bsd.lkm.mk>
