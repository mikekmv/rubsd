# $RuOBSD$

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-fno-builtin-log -fno-builtin-printf

CPPFLAGS+=-I${.CURDIR}/../../include

MAN?=

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}
