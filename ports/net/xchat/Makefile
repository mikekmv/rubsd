# $RuOBSD: Makefile,v 1.2 2005/10/19 19:01:08 form Exp $
# $OpenBSD: Makefile,v 1.49 2005/10/03 20:39:59 couderc Exp $

COMMENT=	"GTK+ IRC client"

MAJOR=			1.8
VERSION=		${MAJOR}.11
DISTNAME=		xchat-${VERSION}
CATEGORIES=		net x11

EXTRACT_SUFX=		.tar.bz2

HOMEPAGE=		http://www.xchat.org/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}files/source/${MAJOR}/ \
			http://xchat.linuxpower.org/files/source/${MAJOR}/

WANTLIB=		c crypto m perl ssl util X11 Xext Xi

LIB_DEPENDS=		gtk.1.2,gdk.1.2::x11/gtk+

MODULES=		converters/libiconv devel/gettext

USE_X11=		Yes
CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--enable-perl --enable-ipv6 --enable-japanese-conv \
			--enable-openssl="/usr" \
			--disable-python --disable-textfe --disable-mmx \
			--disable-zvt --disable-gnome --disable-gdk-pixbuf

CONFIGURE_ENV=		CFLAGS="-I${LOCALBASE}/include" \
			LIBS="-L${LOCALBASE}/lib"

post-install:
	@rm -rf ${WRKSRC}/doc/CVS
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/xchat
	${INSTALL_DATA} ${WRKSRC}/README ${PREFIX}/share/doc/xchat
	${INSTALL_DATA} ${WRKSRC}/doc/* ${PREFIX}/share/doc/xchat

.include <bsd.port.mk>
