# $RuOBSD: Makefile,v 1.2 2002/05/24 04:39:36 form Exp $
# $FreeBSD: ports/irc/ircd-hybrid/Makefile,v 1.17 2001/02/01 10:43:34 phk Exp $

DISTNAME=		ircd-hybrid-6.4.1
CATEGORIES=		net
COMMENT=		"IRC server with many implemented options"
MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=/ircd-hybrid/}
EXTRACT_SUFX=		.tgz

HOMEPAGE=		http://ircd-hybrid.sourceforge.net/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	gnu

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/src/ircd ${PREFIX}/sbin
	${INSTALL_PROGRAM} ${WRKBUILD}/tools/fixklines \
		${PREFIX}/sbin/ircd.fixklines
	${INSTALL_PROGRAM} ${WRKBUILD}/tools/mkpasswd \
		${PREFIX}/sbin/ircd.mkpasswd
	${INSTALL_PROGRAM} ${WRKBUILD}/tools/viconf \
		${PREFIX}/sbin/ircd.viconf
	${INSTALL_SCRIPT} ${WRKSRC}/tools/untabify \
		${PREFIX}/bin/ircd.untabify
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/ircd
	${INSTALL_DATA} ${WRKSRC}/doc/example.conf \
		${PREFIX}/share/examples/ircd
	${INSTALL_DATA} ${WRKSRC}/doc/simple.conf \
		${PREFIX}/share/examples/ircd
	${INSTALL_DATA} ${WRKSRC}/opers.txt ${PREFIX}/share/examples/ircd
	${INSTALL_MAN} ${WRKSRC}/doc/ircd.8 ${PREFIX}/man/man8

.include <bsd.port.mk>
