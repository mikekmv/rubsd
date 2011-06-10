# $RuOBSD: Makefile,v 1.2 2011/06/10 09:17:53 dinar Exp $

PKGNAME=	qutim-plugin-icq-${VERSION}
VERSION=	0.2

MAINTAINER=	dinar@yantel.ru
COMMENT=	ICQ protocol plugin for qutIM

CATEGORIES=             net

MODULES=                x11/qt4
			
USE_GMAKE=              Yes

BUILD_DEPENDS=		net/qutim/qutim

LIB_DEPENDS=		converters/libiconv \
			devel/gettext



WRKDIST=                ${WRKDIR}/${DISTNAME}/plugins/icq

CHECKSUM_FILE=          ${.CURDIR}/../../distinfo

pre-build:
		cd ${WRKSRC} && /usr/local/bin/qmake4

do-install:
		${INSTALL_PROGRAM_DIR} ${PREFIX}/lib/qutim
		${INSTALL_PROGRAM} ${WRKSRC}/libicq.so ${PREFIX}/lib/qutim
	
.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.port.mk>
