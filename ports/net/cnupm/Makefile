# $RuOBSD: Makefile,v 1.6 2004/04/03 04:49:52 form Exp $

COMMENT=		"IP/IPv6 traffic collector"

DISTNAME=		cnupm-3.4
CATEGORIES=		net

HOMEPAGE=		http://pdp-11.org.ru/~form/cnupm

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/files/

MAKE_ENV=		BINDIR=${TRUEPREFIX}/sbin MANDIR=${TRUEPREFIX}/man/cat

FLAVORS=		static
FLAVOR?=

.if ${FLAVOR:Mstatic}
LDSTATIC=		-static
MAKE_ENV+=		LDSTATIC=${LDSTATIC}
.endif

MAKE_FILE=		BSDmakefile

.if !make(fake)
DESTDIRNAME=		/
.endif

.include <bsd.port.mk>
