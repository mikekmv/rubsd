# $OpenBSD$
# $RuOBSD: Makefile,v 1.1.1.1 2002/03/15 17:10:29 grange Exp $
# $NetBSD: Makefile,v 1.1.1.1 2001/12/29 00:22:02 abs Exp $

COMMENT=		"Open Source 68k Macintosh emulator"

DISTNAME=		BasiliskII_src_31052001
PKGNAME=		BasiliskII-0.9.1
CATEGORIES=		emulators
NEED_VERSION=		1.508

HOMEPAGE=		http://www.uni-mainz.de/~bauec002/B2Main.html

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=	http://iphcip1.physik.uni-mainz.de/~cbauer/

LIB_DEPENDS=	gtk.1.2,gdk::x11/gtk+ glib.1.2::devel/glib esd.2::audio/esound

USE_X11=		Yes
USE_GMAKE=		Yes

CONFIGURE_STYLE=	gnu autoconf

WRKDIST=		${WRKDIR}/BasiliskII-0.9
WRKSRC=			${WRKDIR}/BasiliskII-0.9/src/Unix

.include <bsd.port.mk>
