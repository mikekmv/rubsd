# $RuOBSD$

ONLY_FOR_ARCHS=		i386

COMMENT=		"thorough, stand alone memory test"

DISTNAME=		memtest86+-1.27
CATEGORIES=		sysutils

HOMEPAGE=		http://www.memtest.com

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

post-extract:
	@mv -f ${WRKDIST}/stdin.h ${WRKDIST}/stdint.h

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/memtest86+
	${INSTALL_PROGRAM} ${WRKBUILD}/memtest ${PREFIX}/share/memtest86+
	${INSTALL_DATA} ${WRKDIST}/memtest.bin \
	    ${PREFIX}/share/memtest86+/memtest.floppy

.include <bsd.port.mk>
