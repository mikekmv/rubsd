# $RuOBSD$

COMMENT=	"squid analysis report generator"

DISTNAME=	sarg-1.1.1
CATEGORIES=	www net
MASTER_SITES=	http://web.onda.com.br/orso/

HOMEPAGE=	http://web.onda.com.br/orso/

MAINTAINER=	Oleg Safiullin <form@openbsd.org>

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

do-configure:
	@cd ${WRKSRC}; sh configure --enable-bindir=${PREFIX}/bin \
		--enable-sysconfdir=${SYSCONFDIR}/sarg

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
