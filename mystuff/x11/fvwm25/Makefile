# $RuOBSD: Makefile,v 1.8 2010/05/30 17:57:23 form Exp $

COMMENT=		"multiple virtual desktop window manager"

VERSION=		2.5.31
DISTNAME=		fvwm-${VERSION}
PKGNAME=		fvwm25-${VERSION}
CATEGORIES=     	x11

DISTFILES=		fvwm-${VERSION}.tar.bz2 fvwm_icons-20070101.tar.gz:0

HOMEPAGE=		http://www.fvwm.org/

# GPL/BSD-like (badly worded)
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes 
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes 

WANTLIB=		ICE SM X11 Xext Xft Xinerama Xpm Xrender c expat \
			fontconfig freetype m readline

MASTER_SITES=		ftp://ftp.fvwm.org/pub/fvwm/version-2/
MASTER_SITES0=		http://www.fvwm.org/generated/icon_download/

MODULES=		devel/gettext

LIB_DEPENDS=		gtk,gdk:gtk+-*:x11/gtk+ \
			gdk_imlib:imlib-*:graphics/imlib

FLAVORS=		rplay debug
FLAVOR?=

.if ${FLAVOR:L:Mrplay}
LIB_DEPENDS+=		rplay::audio/rplay
CONFIGURE_ARGS+=	--with-rplay-includes="${LOCALBASE}/include" \
			--with-rplay-library="${LOCALBASE}/lib"
.else
CONFIGURE_ARGS+=	--without-rplay-library
.endif

.if ${FLAVOR:L:Mdebug}
CONFIGURE_ARGS+=	--enable-debug-msgs
.endif

USE_X11=		Yes
USE_GMAKE=		Yes

SEPARATE_BUILD=		concurrent
CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--with-intl-includes=${LOCALBASE}/include \
			--with-intl-library=${LOCALBASE}/lib \
			--with-iconv-includes=${LOCALBASE}/include \
			--with-iconv-library=${LOCALBASE}/lib \
			--with-png-includes="${LOCALBASE}/include/libpng" \
			--with-png-library="${LOCALBASE}/lib" \
			--with-imagepath="${LOCALBASE}/share/pixmaps/fvwm" \
			--without-stroke-library --without-gnome --disable-bidi
CONFIGURE_ENV=		LOCALBASE=${LOCALBASE} CPPFLAGS="-I${LOCALBASE}/include"

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/pixmaps/fvwm
	${INSTALL_DATA} ${WRKDIR}/fvwm_icons-20070101/*.xpm ${PREFIX}/share/pixmaps/fvwm
	${INSTALL_DATA} ${WRKSRC}/sample.fvwmrc/system.* ${PREFIX}/share/fvwm
	
.include <bsd.port.mk>
