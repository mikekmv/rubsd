# $RuOBSD: Makefile,v 1.1.1.1 2001/03/04 06:00:52 form Exp $

COMMENT=	"piece of shit"

DISTNAME=	hpt-0.9.7d
DISTFILES=	${DISTNAME}-src.tar.gz
CATEGORIES=	mail
MASTER_SITES=	http://www.physcip.uni-stuttgart.de:8080/fidosoft.husky/

MAINTAINER=	Oleg Safiullin <form@openbsd.ru>

BUILD_DEPENDS=	${LOCALBASE}/lib/libfidoconfigbsd.a::${.CURDIR}/../fidoconf

WRKBUILD=	${WRKSRC}/freebsd
MAKE_FILE=	makefile.bsd

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/hpt ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/hptlink ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/hpttree ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/pktinfo ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/tpkt ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/txt2pkt ${PREFIX}/bin

.include <bsd.port.mk>
