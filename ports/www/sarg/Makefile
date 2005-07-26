# $RuOBSD: Makefile,v 1.3 2002/04/17 09:14:07 form Exp $

COMMENT=		"squid analysis report generator"

DISTNAME=		sarg-2.0.8
CATEGORIES=		www net
MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=sarg/}

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

RUN_DEPENDS=		:bzip2-*:archivers/bzip2

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		--enable-sysconfdir=${SYSCONFDIR}/sarg \
			--enable-bindir=${PREFIX}/bin

post-extract:
	@find ${WRKDIST} -type d |xargs chmod +x  

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/sarg ${PREFIX}/bin
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/sarg/languages
	${INSTALL_DATA} ${WRKBUILD}/languages/* \
		${PREFIX}/share/examples/sarg/languages
	${INSTALL_DATA} ${WRKBUILD}/exclude_codes \
		${PREFIX}/share/examples/sarg
	${INSTALL_DATA} ${WRKBUILD}/sarg.conf \
		${PREFIX}/share/examples/sarg

.include <bsd.port.mk>
