# $RuOBSD$

LKM=	hproc
MAN=

CFLAGS+=-Wall -Werror -Wmissing-prototypes

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}

.include <bsd.lkm.mk>
