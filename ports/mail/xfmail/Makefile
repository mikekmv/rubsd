# $OpenBSD: Makefile,v 1.23 2002/05/03 20:16:45 brad Exp $
# $NetBSD: Makefile,v 1.2 1998/07/04 09:19:04 mycroft Exp $
# $RuOBSD$

COMMENT=		"xforms-based mail application for Unix"

DISTNAME=       	xfmail-1.5.2
CATEGORIES=     	mail x11
NEED_VERSION=		1.522
MASTER_SITES=		ftp://xfmail.ufies.org/pub/xfmail/release/old/1.4.6-3/src/ \
			ftp://cupido.geophys.nat.tu-bs.de/pub/xfmail/release/old/1.4.6-3/src/
EXTRACT_SUFX=		.tar.bz2

HOMEPAGE=		http://www.xfmail.org/

BUILD_DEPENDS=		automake::devel/automake

LIB_DEPENDS=		forms::x11/xforms \
			glib.1.2::devel/glib

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

USE_X11=		Yes
USE_LIBTOOL=		Yes
USE_GMAKE=		Yes
CONFIGURE_STYLE=	autoconf
CONFIGURE_ARGS=		--without-nas-sound
CONFIGURE_ENV=		CPPFLAGS="-DX11FORMS_H" #LDFLAGS="-static"

pre-configure:
	@cd ${WRKSRC} && sh autogen.sh

.include <bsd.port.mk>
