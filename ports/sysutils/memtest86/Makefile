# $RuOBSD: Makefile,v 1.3 2004/10/14 08:26:59 form Exp $

ONLY_FOR_ARCHS=		i386

COMMENT=		"thorough, stand alone memory test"

DISTNAME=		memtest86-3.1a
CATEGORIES=		sysutils

HOMEPAGE=		http://www.memtest86.com

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/

USE_GMAKE=		Yes
CFLAGS=			-fno-stack-protector

FLAVORS=		serial
FLAVOR?=

.if ${FLAVOR:L:Mserial}
CFLAGS+=		-DSERIAL_CONSOLE_DEFAULT=1
.endif

BINMODE=		444

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/memtest86
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/memtest86
	${INSTALL_PROGRAM} ${WRKBUILD}/memtest ${PREFIX}/share/memtest86
	${INSTALL_DATA} ${WRKDIST}/memtest.bin \
	    ${PREFIX}/share/memtest86/memtest.floppy

.include <bsd.port.mk>
