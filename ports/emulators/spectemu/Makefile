# $RuOBSD: Makefile,v 1.4 2002/02/07 22:31:53 grange Exp $

COMMENT=		"ZX Spectrum 48k emulator"

VERSION=		0.94
DISTNAME=		spectemu-${VERSION}
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

CONFIGURE_STYLE=	gnu

MAKE_ENV=		SYSCONFDIR=${SYSCONFDIR}

post-patch:
	@sed 's!__SYSCONFDIR__!${SYSCONFDIR}!' ${WRKSRC}/README > \
	    ${WRKSRC}/README.new
	@mv ${WRKSRC}/README.new ${WRKSRC}/README

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/xspect ${PREFIX}/bin
	${INSTALL_MAN} ${WRKSRC}/xspect.1 ${PREFIX}/man/man1
	${INSTALL_DATA_DIR} ${PREFIX}/share/spectemu
	${INSTALL_DATA} ${WRKSRC}/spectkey.gif ${PREFIX}/share/spectemu
	${INSTALL_DATA} ${WRKSRC}/specsinc.xpm ${PREFIX}/share/spectemu
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/spectemu
	${INSTALL_DATA} ${WRKSRC}/example.cfg ${PREFIX}/share/examples/spectemu
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/spectemu
	${INSTALL_DATA} ${WRKSRC}/README ${PREFIX}/share/doc/spectemu

.include <bsd.port.mk>
