
PKGNAME=	qutim-${VERSION}
VERSION=	0.2.0

COMMENT=	A Qt4-based multiprotocol IM client with advanced features


MODULES=                devel/cmake \
			x11/qt4
			
CONFIGURE_STYLE=        cmake
USE_GMAKE=              Yes
USE_X11=                Yes

LIB_DEPENDS=		converters/libiconv \
			devel/gettext
BUILD_DEPENDS=	 	x11/xscreensaver 

CHECKSUM_FILE=${.CURDIR}/../distinfo

CONFIGUREE_ARGS+=	-DUNIX:BOOL=1 -DBSD:BOOL=1



.include <bsd.port.mk>
