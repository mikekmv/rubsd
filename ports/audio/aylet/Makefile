# $RuOBSD: Makefile,v 1.1.1.1 2002/02/01 03:39:36 grange Exp $

COMMENT=		"ZX Spectrum AY music player"

VERSION=		0.2
DISTNAME=		aylet-${VERSION}
CATEGORIES=		audio
NEED_VERSION=		1.504

HOMEPAGE=		http://rus.members.beeb.net/aylet.html

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		ftp://ftp.ibiblio.org/pub/Linux/apps/sound/players/

FLAVORS=no_x11
FLAVOR?=

.if ${FLAVOR:L:Mno_x11}
ALL_TARGET=	aylet
.else
LIB_DEPENDS=	gtk.1.2,gdk.1.2::x11/gtk+
USE_X11=	Yes
.endif

.include <bsd.port.mk>
