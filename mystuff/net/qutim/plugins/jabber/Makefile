# $RuOBSD: Makefile,v 1.2 2011/06/10 09:17:53 dinar Exp $

PKGNAME=	qutim-plugin-jabber-${VERSION}
VERSION=	0.2

MAINTAINER=	dinar@yantel.ru
COMMENT=	Jabber protocol plugin for qutIM

CATEGORIES=             net

MODULES=                x11/qt4 \
			devel/cmake
			
CONFIGURE_STYLE=        cmake


BUILD_DEPENDS=		net/qutim/qutim

LIB_DEPENDS=		converters/libiconv \
			devel/gettext \
			devel/libidn



WRKDIST=                ${WRKDIR}/${DISTNAME}/plugins/jabber

CHECKSUM_FILE=          ${.CURDIR}/../../distinfo

#pre-build:
#		cd ${WRKSRC} && /usr/local/bin/qmake4

do-install:
		${INSTALL_PROGRAM_DIR} ${PREFIX}/lib/qutim
		${INSTALL_PROGRAM} ${WRKSRC}/libjabber.so ${PREFIX}/lib/qutim
	
.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.port.mk>



