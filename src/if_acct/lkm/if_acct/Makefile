# $RuOBSD$

LKM=	if_acct
SRCS=	if_acct.c if_acct_lkm.c

MAN=

INCDIR=	/usr/local/include/net

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}

afterinstall: ${INCDIR}
	install -o ${BINOWN} -g ${BINGRP} -m ${NONBINMODE} \
	    ${.CURDIR}/../../include/net/if_acct.h ${INCDIR}

${INCDIR}:
	install -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d ${INCDIR}

.include <bsd.lkm.mk>
