# $RuOBSD: Makefile,v 1.13 2011/06/10 09:17:54 dinar Exp $

COMMENT=		plugin-based instant messenger
DISTNAME=		sim-im-devel-0.9.5.2467
PKGNAME=		${DISTNAME}p0
EXTRACT_SUFX=		.tar.bz2
CATEGORIES=		net

SHARED_LIBS=		sim	0.0

HOMEPAGE=		http://sim-im.org/

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		http://pdp-11.org.ru/~form/files/sim-im-devel/

MODULES=		converters/libiconv x11/qt3

BUILD_DEPENDS=		archivers/zip
LIB_DEPENDS=		graphics/jpeg graphics/png \
			textproc/libxslt

FLAVORS=		kde
FLAVOR?=

USE_X11=		Yes
USE_GMAKE=		Yes
USE_LIBTOOL=		Yes

CONFIGURE_STYLE=	autoconf
AUTOCONF_VERSION=	2.62
CONFIGURE_ENV=		LDFLAGS="-L${LOCALBASE}/lib" \
			UIC_PATH="${MODQT_UIC}"
CONFIGURE_ARGS=		--with-qt-includes=${MODQT_INCDIR} \
			--with-qt-libraries=${MODQT_LIBDIR} \
			--disable-debug

WANTLIB=		GL ICE SM X11 Xau Xcursor Xdamage Xdmcp Xext Xfixes \
			Xft Xi Xinerama Xmu Xrandr Xrender Xt Xxf86vm c \
			crypto drm expat iconv fontconfig freetype lcms m mng \
			png pthread qt-mt ssl stdc++ xml2 z

.if ${FLAVOR:L:Mkde}
LIB_DEPENDS+=		DCOP,kdecore,kdefx,kdesu,kdeui,kio,kwalletclient::x11/kde/libs3
MODKDE_VERSION=		3.5.8
WANTLIB+=		art_lgpl_2 fam idn intl rpcsvc util
.else
CONFIGURE_ARGS+=	--disable-kde
.endif

PORTHOME=		${WRKBUILD}

.include <bsd.port.mk>
