# $RuOBSD: Makefile,v 1.3 2006/01/26 14:59:49 form Exp $

ONLY_FOR_ARCHS=		amd64 i386

COMMENT=		"thorough, stand alone memory test"

DISTNAME=		memtest86+-${VERSION}
VERSION=		1.70
CATEGORIES=		sysutils

HOMEPAGE=		http://www.memtest.org

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/download/${VERSION}/

USE_GMAKE=		Yes
MAKE_ENV=		CCFLAGS="${CCFLAGS}"
CCFLAGS=		-fno-stack-protector

FLAVORS=		serial
FLAVOR?=

.if ${FLAVOR:L:Mserial}
CCFLAGS+=		-DSERIAL_CONSOLE_DEFAULT=1
.endif

BINMODE=		444

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/memtest86+
	${INSTALL_PROGRAM} ${WRKBUILD}/memtest ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/memtest.bin \
	    ${PREFIX}/share/memtest86+/memtest.floppy

.include <bsd.port.mk>
