# $RuOBSD: Makefile,v 1.3 2002/02/11 22:54:34 grange Exp $

COMMENT=		"Free Unix Spectrum Emulator"

VERSION=		0.4.1
DISTNAME=		fuse-${VERSION}
CATEGORIES=		emulators
NEED_VERSION=		1.504

HOMEPAGE=		http://www.srcf.ucam.org/~pak21/spectrum/fuse.html

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

#GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=	http://www.srcf.ucam.org/~pak21/spectrum/ \
		ftp://ftp.worldofspectrum.org/pub/sinclair/emulators/unix/

USE_X11=		Yes

CONFIGURE_STYLE=	gnu autoconf

FLAVORS=gtk
FLAVOR?=

.if ${FLAVOR:L:Mgtk}
LIB_DEPENDS=		gtk.1.2,gdk.1.2::x11/gtk+
.else
CONFIGURE_ARGS=		--without-gtk
.endif

post-extract:
	@ln -s ${FILESDIR}/sunsound.c ${WRKSRC}
	@ln -s ${FILESDIR}/sunsound.h ${WRKSRC}

DOCS=	AUTHORS THANKS

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/fuse
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${FILE} ${PREFIX}/share/doc/fuse
.endfor

.include <bsd.port.mk>
