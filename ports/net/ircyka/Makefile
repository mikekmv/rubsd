# $RuOBSD: Makefile,v 1.9 2006/04/25 21:17:54 form Exp $

COMMENT=		"IRC bot for Hybrid IRC servers"
DISTNAME=		ircyka-6.6.6
PKGNAME=		${DISTNAME}p2
CATEGORIES=		net

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c

MASTER_SITES=		http://pdp-11.org.ru/~form/ircyka/

DESTDIRNAME=		none

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/ircyka
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/doc/ircyka
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/ircyka
	${INSTALL_DATA} ${WRKDIST}/etc/ircyka.conf \
	    ${PREFIX}/share/examples/ircyka

.include <bsd.port.mk>
