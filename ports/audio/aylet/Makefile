# $RuOBSD$

COMMENT=		"ZX Spectrum AY music player"

DISTNAME=		aylet-0.2
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

DOCS=	COPYING ChangeLog NEWS README TODO
post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/aylet
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${FILE} ${PREFIX}/share/doc/aylet
.endfor

.include <bsd.port.mk>
