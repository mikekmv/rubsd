# $RuOBSD$

COMMENT=		"Tunnel Setup Protocol Client for Freenet6"

DISTNAME=		tspc-2.1.1-src
PKGNAME=		${DISTNAME:S/-src//}
CATEGORIES=		net

HOMEPAGE=		http://www.hexago.com/

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c

MASTER_SITES=		${HOMEPAGE}/files/
EXTRACT_SUFX=		.tgz

MAKE_ENV=		target=openbsd
CFLAGS+=		-DSYSCONFDIR=\"${SYSCONFDIR}\"

WRKDIST=		${WRKDIR}/tspc2

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/bin/tspc ${PREFIX}/sbin/
	${INSTALL_MAN} ${WRKDIST}/man/man5/tspc.conf.5 ${PREFIX}/man/man5
	${INSTALL_MAN} ${WRKDIST}/man/man8/tspc.8 ${PREFIX}/man/man8/
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/tspc
	${INSTALL_DATA} ${WRKBUILD}/bin/tspc.conf.sample \
	    ${PREFIX}/share/examples/tspc

.include <bsd.port.mk>
