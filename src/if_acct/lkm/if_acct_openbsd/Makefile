# $RuOBSD$

LKM=	if_acct
SRCS=	if_acct.c if_acct_lkm.c

MAN=

.PATH:	${.CURDIR}/../if_acct

load:	${COMBINED}
	modload -o ${LKM} -e ${LKM}_lkmentry ${COMBINED}

.include <bsd.lkm.mk>
