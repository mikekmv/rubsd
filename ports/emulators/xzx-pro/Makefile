# $RuOBSD: Makefile,v 1.4 2002/03/18 13:42:25 grange Exp $

COMMENT=		"ZX Spectrum 48k/128k/+3 emulator"

VERSION=		4.1b1
DISTNAME=		xzx-pro-${VERSION}-eval
PKGNAME=		xzx-pro-${VERSION}
CATEGORIES=		emulators
NEED_VERSION=		1.504

HOMEPAGE=		http://www.zx-spectrum.net/xzx/

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

PERMIT_PACKAGE_CDROM=	"may not be distributed or modified"
PERMIT_PACKAGE_FTP=	"may not be distributed or modified"
PERMIT_DISTFILES_CDROM=	"may not be distributed or modified"
PERMIT_DISTFILES_FTP=	"may not be distributed or modified"

MASTER_SITES=		http://xzx.purespace.de/xzx/

USE_X11=		Yes
USE_MOTIF=		any

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--enable-evaluation --with-oss

WRKDIST=		${WRKDIR}/${PKGNAME}

pre-configure:
	cd ${WRKSRC} && autoconf

DOCS=	COPYRIGHT FAQ README
post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/xzx-pro
	${INSTALL_DATA} ${WRKSRC}/doc/Xzx.ad ${PREFIX}/share/xzx-pro/Xzx.ad
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/xzx-pro
.for FILE in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/doc/${FILE} ${PREFIX}/share/doc/xzx-pro
.endfor
	${INSTALL_MAN} ${WRKSRC}/doc/xzx.man ${PREFIX}/man/man1/xzx.1

.include <bsd.port.mk>
