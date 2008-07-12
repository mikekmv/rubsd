# $RuOBSD$

COMMENT=	"GTK+ IRC client"

VERSION=		1.8
MINOR=			11
DISTNAME=		xchat-${VERSION}.${MINOR}
PKGNAME=		xchat1-${VERSION}.${MINOR}
CATEGORIES=		net x11

EXTRACT_SUFX=		.tar.bz2

HOMEPAGE=		http://www.xchat.org/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}files/source/${VERSION}/ \
			http://xchat.linuxpower.org/files/source/${VERSION}/

WANTLIB=		c crypto m perl ssl util X11 Xext Xi

LIB_DEPENDS=		gtk,gdk::x11/gtk+

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
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/xchat1
	${INSTALL_DATA} ${WRKSRC}/README ${PREFIX}/share/doc/xchat1
	${INSTALL_DATA} ${WRKSRC}/doc/* ${PREFIX}/share/doc/xchat1

.include <bsd.port.mk>
