# $RuOBSD: Makefile,v 1.2 2002/01/31 14:17:12 grange Exp $

COMMENT=		"ZX Spectrum 48k emulator"

DISTNAME=		spectemu-0.94
CATEGORIES=		emulators
NEED_VERSION=		1.504

HOMEPAGE=		http://www.inf.bme.hu/~mszeredi/spectemu/

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		http://www.inf.bme.hu/~mszeredi/spectemu/

USE_X11=		Yes

CONFIGURE_STYLE=	gnu dest

DOCS=	COPYING ChangeLog README README.DOS README.Z80 TODO
post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/spectemu
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${FILE} ${PREFIX}/share/doc/spectemu
.endfor

.include <bsd.port.mk>
