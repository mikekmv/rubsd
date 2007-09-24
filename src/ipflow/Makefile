# $RuOBSD: Makefile,v 1.2 2005/11/24 17:01:17 form Exp $

SUBDIR=	lkm bin include

.if defined(NO_PROPOLICE) && ${NO_PROPOLICE:U} == YES
MAKE_FLAGS+=NO_PROPOLICE=Yes
.endif

.include <bsd.subdir.mk>
