# $RuOBSD$

COMMENT=		"proxy-cache-servers logfiles analizer"

DISTNAME=		calamaris-2.42
CATEGORIES=		www sysutils
NEED_VERSION=		1.500

HOMEPAGE=		http://cord.de/tools/squid/calamaris/	
MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		http://cord.de/tools/squid/calamaris/

NO_BUILD=		Yes

DOCS=			CHANGES COPYRIGHT EXAMPLES README

do-install:
	${INSTALL_SCRIPT} ${WRKSRC}/calamaris ${PREFIX}/bin
	${INSTALL_MAN} ${WRKSRC}/calamaris.1 ${PREFIX}/man/man1
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/calamaris
.for i in ${DOCS}
	${INSTALL_DATA} ${WRKSRC}/${i} ${PREFIX}/share/doc/calamaris
.endfor

.include <bsd.port.mk>
