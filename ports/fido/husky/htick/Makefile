# $RuOBSD: Makefile,v 1.2 2001/10/31 06:52:51 form Exp $

COMMENT=	"piece of shit"

DISTNAME=	htick-0.9a
DISTFILES=	${DISTNAME}-src.tar.gz
CATEGORIES=	mail
MASTER_SITES=	http://www.physcip.uni-stuttgart.de:8080/fidosoft.husky/

BUILD_DEPENDS=	${LOCALBASE}/lib/libfidoconfigbsd.a::${.CURDIR}/../fidoconf

WRKBUILD=	${WRKSRC}/bsd
MAKE_FILE=	makefile.bsd

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/htick ${PREFIX}/bin

.include <bsd.port.mk>
