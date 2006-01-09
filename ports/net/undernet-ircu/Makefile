# $RuOBSD$

COMMENT=		"Undernet IRC daemon"
DISTNAME=		ircu${VERSION}
PKGNAME=		undernet-ircu-${VERSION}
VERSION=		2.10.11.07
CATEGORIES=		net

HOMEPAGE=		http://coder-com.undernet.org/

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c

MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=undernet-ircu/}

SEPARATE_BUILD=		concurrent
CONFIGURE_STYLE=	gnu

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/ircd/ircd ${PREFIX}/libexec/ircd
	${INSTALL_PROGRAM} ${WRKBUILD}/ircd/chkconf ${PREFIX}/sbin/ircd-chkconf
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/undernet-ircu
	${INSTALL_DATA} ${WRKDIST}/doc/example.conf \
	    ${PREFIX}/share/examples/undernet-ircu/ircd.conf.sample
	${INSTALL_MAN} ${WRKDIST}/doc/ircd.8 ${PREFIX}/man/man8

.include <bsd.port.mk>
