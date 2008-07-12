# $RuOBSD$

COMMENT=		"Qt GUI for licq"
CATEGORIES=		net x11
PKGNAME=		licq-qt-gui-${VERSION}p0

WANTLIB=		GL ICE SM X11 Xau Xcursor Xdmcp Xext Xfixes Xft \
			Xinerama Xmu Xrandr Xrender Xss Xt c expat m pthread z

MODULES=		x11/qt3
MODQT_MT=		Yes

LIB_DEPENDS=		jpeg::graphics/jpeg \
			png::graphics/png

USE_X11=		Yes
USE_GMAKE=		Yes
USE_LIBTOOL=		Yes

CONFIGURE_STYLE=	gnu
CONFIGURE_ENV=		LDFLAGS="-L${LOCALBASE}/lib"
CONFIGURE_ARGS=		--with-qt-includes=${LOCALBASE}/include/X11/qt3 \
			--with-qt-libraries=${LOCALBASE}/lib/qt3 \
			--with-qt-moc=${LOCALBASE}/bin/moc3-mt

WRKDIST=		${WRKDIR}/${DISTNAME}/plugins/qt-gui

CHECKSUM_FILE=		${.CURDIR}/../../distinfo

.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.port.mk>
