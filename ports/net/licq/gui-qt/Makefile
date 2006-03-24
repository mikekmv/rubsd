# $RuOBSD: Makefile,v 1.5 2006/02/17 20:42:33 form Exp $

COMMENT=		"Qt GUI for licq"

PKGNAME=		licq-gui-qt-${VERSION}

WANTLIB=		ICE SM X11 Xext Xss c m pthread z

MODULES=		x11/qt3
MODQT_MT=		Yes

RUN_DEPENDS=		licq::${.CURDIR}/../licq
LIB_DEPENDS=		jpeg.62::graphics/jpeg \
			png.2::graphics/png

USE_X11=		Yes
USE_GMAKE=		Yes
USE_LIBTOOL=		Yes
CONFIGURE_STYLE=	gnu
CONFIGURE_ENV=		LDFLAGS="-L${LOCALBASE}/lib"
CONFIGURE_ARGS=		--with-qt-includes=${LOCALBASE}/include/X11/qt3 \
			--with-qt-libraries=${LOCALBASE}/lib/qt3 \
			--with-qt-moc=${LOCALBASE}/bin/moc3

WRKDIST=		${WRKDIR}/${DISTNAME}/plugins/qt-gui
WRKSRC=			${WRKDIR}/${DISTNAME}/plugins/qt-gui

.include <bsd.port.mk>
