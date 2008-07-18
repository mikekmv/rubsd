# $RuOBSD: Makefile,v 1.2 2008/07/14 17:41:48 form Exp $

COMMENT=		CVS pserver wrapper
DISTNAME=		cvsd-1.0.15
CATEGORIES=		devel

HOMEPAGE=		http://ch.tudelft.nl/~arthur/cvsd/

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c

MASTER_SITES=		${HOMEPAGE}

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		--with-libwrap
SEPARATE_BUILD=		concurrent

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/cvsd
	${INSTALL_DATA} ${WRKDIST}/cvsd.conf-dist \
	    ${PREFIX}/share/examples/cvsd/cvsd.conf

.include <bsd.port.mk>
