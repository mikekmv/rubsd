# $RuOBSD: Makefile,v 1.3 2004/03/19 03:26:25 form Exp $

COMMENT=		"IP/IPv6 traffic collector"

DISTNAME=		cnupm-3.2
CATEGORIES=		net

HOMEPAGE=		http://pdp-11.org.ru/~form/cnupm

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/files/

FLAVORS=		static
FLAVOR?=

.if ${FLAVOR:Mstatic}
LDSTATIC=		-static
.endif

MAKE_ENV=		LDSTATIC=${LDSTATIC}

.include <bsd.port.mk>
