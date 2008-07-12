# $RuOBSD$

COMMENT=		"DEC PRO 350 emulator based on SIMH"

DISTNAME=		xhomer-9-16-06
PKGNAME=		xhomer-9.16.06
EXTRACT_SUFX=		.tgz
CATEGORIES=		emulators

HOMEPAGE=		http://xhomer.isani.org/xhomer/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		X11 Xext Xxf86dga c m

MASTER_SITES=		${HOMEPAGE}

USE_X11=		Yes
USE_GMAKE=		Yes
MAKE_ENV=		X11DIR=${X11BASE}
ALL_TARGET=		xhomer

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/xhomer ${PREFIX}/bin
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/xhomer
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/xhomer/html
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/doc/xhomer
	${INSTALL_DATA} ${WRKDIST}/DOC/* ${PREFIX}/share/doc/xhomer/html
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/xhomer
	${INSTALL_DATA} ${WRKDIST}/xhomer.cfg ${PREFIX}/share/examples/xhomer

.include <bsd.port.mk>
