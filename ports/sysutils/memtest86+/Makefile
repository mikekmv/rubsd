# $RuOBSD: Makefile,v 1.1.1.1 2004/10/20 07:55:04 form Exp $

ONLY_FOR_ARCHS=		i386

COMMENT=		"thorough, stand alone memory test"

DISTNAME=		memtest86+-${VERSION}
VERSION=		1.27
CATEGORIES=		sysutils

HOMEPAGE=		http://www.memtest.org

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/download/${VERSION}/

USE_GMAKE=		Yes
CFLAGS=			-fno-stack-protector

FLAVORS=		serial
FLAVOR?=

.if ${FLAVOR:L:Mserial}
CFLAGS+=		-DSERIAL_CONSOLE_DEFAULT=1
.endif

BINMODE=		444

post-extract:
	@mv -f ${WRKDIST}/stdin.h ${WRKDIST}/stdint.h

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/memtest86+
	${INSTALL_PROGRAM} ${WRKBUILD}/memtest ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/memtest.bin \
	    ${PREFIX}/share/memtest86+/memtest.floppy

.include <bsd.port.mk>
