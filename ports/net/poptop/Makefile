# $RuOBSD$

COMMENT=		"Microsoft VPN compatible PPTP server"

DISTNAME=		pptpd-${VERSION}
PKGNAME=		poptop-${VERSION}
VERSION=		1.0.1
CATEGORIES=		net

HOMEPAGE=		http://www.snapgear.com/vpn/pptp.html

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		ftp://ftp.snapgear.com/poptop/

SEPARATE_BUILD=		concurrent

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		--with-libwrap

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/poptop
	${INSTALL_DATA} ${WRKDIST}/samples/pptpd.conf \
	    ${PREFIX}/share/examples/poptop

.include <bsd.port.mk>
