# $RuOBSD: Makefile,v 1.5 2004/04/02 10:20:20 form Exp $

COMMENT=		"IP/IPv6 traffic collector"

DISTNAME=		cnupm-3.3
CATEGORIES=		net

HOMEPAGE=		http://pdp-11.org.ru/~form/cnupm

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/files/

FLAVORS=		static
FLAVOR?=

.if ${FLAVOR:Mstatic}
LDSTATIC=		-static
MAKE_ENV=		LDSTATIC=${LDSTATIC}
.endif

.include <bsd.port.mk>
