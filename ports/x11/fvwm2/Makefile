# $OpenBSD: Makefile,v 1.37 2005/07/29 05:01:49 pvalchev Exp $
# $RuOBSD: Makefile,v 1.1.1.1 2005/11/18 23:44:27 form Exp $

COMMENT=		"multiple virtual desktop window manager"

VERSION=		2.5.14
DISTNAME=		fvwm-${VERSION}
CATEGORIES=     	x11

DISTFILES=		fvwm-${VERSION}.tar.bz2 fvwm_icons.tgz:0

HOMEPAGE=		http://www.fvwm.org/

# GPL/BSD-like (badly worded)
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes 
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes 

WANTLIB=		ICE SM X11 Xext Xft Xinerama Xpm Xrender expat \
			fontconfig freetype m readline

MASTER_SITES=		ftp://ftp.fvwm.org/pub/fvwm/version-2/
MASTER_SITES0=		http://www.fvwm.org/generated/icon_download/
MASTER_SITES1=		http://www.fvwm.org/generated/sound_download/

MODULES=		devel/gettext

LIB_DEPENDS=		gtk.1.2:gtk+-*:x11/gtk+ \
			gdk_imlib.19.:imlib-*:graphics/imlib

FLAVORS=		png rplay debug
FLAVOR?=

.if ${FLAVOR:L:Mpng}
LIB_DEPENDS+=		png.2.::graphics/png
CONFIGURE_ARGS+=	--with-png-includes="${LOCALBASE}/include/libpng" \
			--with-png-library="${LOCALBASE}/lib"
.else
CONFIGURE_ARGS+=	--without-png
.endif

.if ${FLAVOR:L:Mrplay}
LIB_DEPENDS+=		rplay.0.::audio/rplay
CONFIGURE_ARGS+=	--with-rplay-includes="${LOCALBASE}/include" \
			--with-rplay-library="${LOCALBASE}/lib"
.else
CONFIGURE_ARGS+=	--without-rplay
.endif

.if ${FLAVOR:L:Mdebug}
CONFIGURE_ARGS+=	--enable-debug-msgs
.endif

USE_X11=		Yes

SEPARATE_BUILD=		concurrent
CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--with-intl-includes=${LOCALBASE}/include \
			--with-intl-library=${LOCALBASE}/lib \
			--with-iconv-includes=${LOCALBASE}/include \
			--with-iconv-library=${LOCALBASE}/lib \
			--without-stroke --without-gnome --without-fribidi
CONFIGURE_ENV=		LOCALBASE=${LOCALBASE}

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/pixmaps/fvwm
	${INSTALL_DATA} ${WRKDIR}/fvwm_icons/*.xpm ${PREFIX}/share/pixmaps/fvwm/
	${INSTALL_DATA} ${WRKSRC}/sample.fvwmrc/system.* ${PREFIX}/share/fvwm
	
.include <bsd.port.mk>
