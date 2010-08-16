# $RuOBSD$

PKGNAME=	qutim-plugin-jabber-${VERSION}
VERSION=	0.2

MAINTAINER=	dinar@yantel.ru
COMMENT=	Jabber protocol plugin for qutIM

CATEGORIES=             net

MODULES=                x11/qt4 \
			devel/cmake
			
CONFIGURE_STYLE=        cmake


BUILD_DEPENDS=		::net/qutim/qutim

LIB_DEPENDS=		iconv.>=6::converters/libiconv \
			intl.>=5:gettext->=0.10.38:devel/gettext \
			idn.>=16::devel/libidn



WRKDIST=                ${WRKDIR}/${DISTNAME}/plugins/jabber

CHECKSUM_FILE=          ${.CURDIR}/../../distinfo

#pre-build:
#		cd ${WRKSRC} && /usr/local/bin/qmake4

do-install:
		${INSTALL_PROGRAM_DIR} ${PREFIX}/lib/qutim
		${INSTALL_PROGRAM} ${WRKSRC}/libjabber.so ${PREFIX}/lib/qutim
	
.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.port.mk>



