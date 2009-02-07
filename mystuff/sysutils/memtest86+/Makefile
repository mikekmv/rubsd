# $RuOBSD: Makefile,v 1.3 2009/02/07 10:28:03 form Exp $

ONLY_FOR_ARCHS=		amd64 i386

COMMENT=		"thorough, stand alone memory test"

VERSION=		2.11
DISTNAME=		memtest86+-${VERSION}
CATEGORIES=		sysutils

HOMEPAGE=		http://www.memtest.org

# GPLv2
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/download/${VERSION}/

USE_GMAKE=		Yes
NO_REGRESS=		Yes
BINMODE=		444

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${FILESDIR}/README.OpenBSD ${PREFIX}/share/memtest86+
	${INSTALL_PROGRAM} ${WRKBUILD}/memtest ${PREFIX}/share/memtest86+
	${INSTALL_PROGRAM} ${WRKBUILD}/memtest_s \
	    ${PREFIX}/share/memtest86+/memtest-serial
	${INSTALL_DATA} ${WRKDIST}/memtest.bin \
	    ${PREFIX}/share/memtest86+/memtest.floppy
	${INSTALL_DATA} ${WRKDIST}/memtest_s.bin \
	    ${PREFIX}/share/memtest86+/memtest-serial.floppy

.include <bsd.port.mk>
