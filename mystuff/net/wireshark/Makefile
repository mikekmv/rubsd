# $RuOBSD $

DISTNAME=	wireshark-${VERSION}
VERSION=	1.6.5
CATEGORIES=	net
MASTER_SITES=	http://www.wireshark.org/download/src/ \
    		http://wireshark.osmirror.nl/download/src/ \
		http://ftp.uni-kl.de/pub/wireshark/src/ \
		http://wireshark.askapache.com/download/src/ \
		http://wireshark.dsmirror.nl/download/src/ \
		ftp://ftp.uni-kl.de/pub/wireshark/src/

EXTRACT_SUFX=		.tar.bz2

MAINTAINER=	dinar@yantel.ru
COMMENT?=	A powerful network analyzer/capture tool

SHARED_LIBS +=	wireshark                 2.1
SHARED_LIBS +=	wiretap                   1.1
SHARED_LIBS +=	wsutil                    1.0


PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	gnu


CONFIGURE_ARGS+=	--with-ssl  \
			--enable-threads \
			--with-pcre \
			--enable-packet-editor \
			--enable-setuid-install \
			--with-krb5=/usr/include/kerberosV \
			--with-portaudio

BUILD_DEPENDS+=		lang/lua \
			audio/portaudio-svn \
			devel/pcre \
			x11/gtk+2 \
			devel/pango \
			graphics/cairo \
			devel/atk \
			devel/gettext \
			graphics/gdk-pixbuf2 \
			graphics/png \
			devel/glib2
USE_GMAKE=	yes

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/applications
	${INSTALL_DATA} ${WRKSRC}/debian/wireshark.desktop ${PREFIX}/share/applications
	${INSTALL_DATA_DIR} ${PREFIX}/share/icons/hicolor/16x16/apps
	${INSTALL_DATA_DIR} ${PREFIX}/share/icons/hicolor/32x32/apps
	${INSTALL_DATA_DIR} ${PREFIX}/share/icons/hicolor/48x48/apps
	${INSTALL_DATA} ${WRKSRC}/image/hi16-app-wireshark.png ${PREFIX}/share/icons/hicolor/16x16/apps
	${INSTALL_DATA} ${WRKSRC}/image/hi32-app-wireshark.png ${PREFIX}/share/icons/hicolor/32x32/apps
	${INSTALL_DATA} ${WRKSRC}/image/hi48-app-wireshark.png ${PREFIX}/share/icons/hicolor/48x48/apps

.include <bsd.port.mk>

