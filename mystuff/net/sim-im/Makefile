# $RuOBSD: Makefile,v 1.1.1.1 2008/07/12 19:00:53 form Exp $

COMMENT=		plugin-based instant messenger
VERSION=		0.9.4.3
DISTNAME=		sim-${VERSION}
PKGNAME=		sim-im-${VERSION}
EXTRACT_SUFX=		.tar.bz2
CATEGORIES=		net

SHARED_LIBS=		sim	0.0

HOMEPAGE=		http://sim-im.org/

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		GL ICE SM X11 Xau Xcursor Xdamage Xdmcp Xext Xfixes \
			Xft Xi Xinerama Xmu Xrandr Xrender Xt Xxf86vm c \
			crypto drm expat fontconfig freetype m pthread ssl \
			stdc++ z

MASTER_SITES=		${MASTER_SITE_BERLIOS:=sim-im/}

MODULES=		converters/libiconv x11/qt3
MODQT_MT=		Yes

BUILD_DEPENDS=		:zip->=2.3:archivers/zip
LIB_DEPENDS=		jpeg::graphics/jpeg xslt::textproc/libxslt

USE_X11=		Yes
USE_GMAKE=		Yes
USE_LIBTOOL=		Yes

CONFIGURE_STYLE=	autoconf
CONFIGURE_ENV=		LDFLAGS="-L${LOCALBASE}/lib" \
			UIC_PATH="${LOCALBASE}/bin/uic3-mt"
CONFIGURE_ARGS=		--with-qt-includes=${LOCALBASE}/include/X11/qt3 \
			--with-qt-libraries=${LOCALBASE}/lib/qt3 \
			--enable-mt --disable-debug --disable-kde
AUTOCONF_VERSION=	2.59

PORTHOME=		${WRKBUILD}

.include <bsd.port.mk>
