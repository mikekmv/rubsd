# $RuOBSD: Makefile,v 1.1.1.1 2004/10/27 06:32:39 form Exp $

OPSYS!=	uname

.if ${OPSYS} == FreeBSD
SUBDIR=	if_acct_freebsd
.endif

.if ${OPSYS} == OpenBSD
SUBDIR=	if_acct_openbsd
.endif

.include <bsd.subdir.mk>
