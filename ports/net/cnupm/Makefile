# $RuOBSD: Makefile,v 1.10 2004/04/24 04:07:43 form Exp $

COMMENT=		"IP/IPv6 traffic collector"

DISTNAME=		cnupm-3.5
CATEGORIES=		net

HOMEPAGE=		http://pdp-11.org.ru/~form/cnupm

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/files/

MAKE_ENV=		BINDIR=${TRUEPREFIX}/sbin MANDIR=${TRUEPREFIX}/man/cat
COPTS=			-DCNUPM_USER=\"_cnupm\"

FLAVORS=		static
FLAVOR?=

.if ${FLAVOR:Mstatic}
LDSTATIC=		-static
MAKE_ENV+=		LDSTATIC=${LDSTATIC}
.endif

.if !make(fake) && !make(install) && !make(package)
DESTDIRNAME=		/
.endif

.include <bsd.port.mk>
