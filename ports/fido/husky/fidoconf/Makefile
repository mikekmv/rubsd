# $RuOBSD: Makefile,v 1.2 2001/10/31 06:45:38 form Exp $

COMMENT=	"piece of shit"

DISTNAME=	fidoconf-0.10c
DISTFILES=	${DISTNAME}-src.tar.gz
CATEGORIES=	mail
MASTER_SITES=	http://www.physcip.uni-stuttgart.de:8080/fidosoft.husky/

MAKE_FILE=	makefile.bsd

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

BUILD_DEPENDS=	${LOCALBASE}/lib/libsmapibsd.a::${.CURDIR}/../smapi

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/fconf2msged ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/fconf2aquaed ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/fconf2fidogate ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/fconf2golded ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/fconf2squish ${PREFIX}/bin
	${INSTALL_PROGRAM} ${WRKBUILD}/dumpfcfg ${PREFIX}/bin
	${INSTALL_DATA_DIR} ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/adcase.h ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/common.h ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/dirlayer.h ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/fidoconf.h ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/typesize.h ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/xstr.h ${PREFIX}/include/fidoconf
	${INSTALL_DATA} ${WRKBUILD}/libfidoconfigbsd.a ${PREFIX}/lib

.include <bsd.port.mk>
