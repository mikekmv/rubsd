# $RuOBSD$

DISTNAME=	smapi-1.6.4b
DISTFILES=	${DISTNAME}-src.tar.gz
CATEGORIES=	mail
MASTER_SITES=	http://www.physcip.uni-stuttgart.de:8080/fidosoft.husky/

MAINTAINER=	Oleg Safiullin <form@openbsd.ru>

MAKE_FILE=	makefile.bsd
ALL_TARGET=	libsmapibsd.a

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/api_brow.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/compiler.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/ffind.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/msgapi.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/patmat.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/prog.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/progprot.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/stamp.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/typedefs.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/unused.h ${PREFIX}/include/smapi
	${INSTALL_DATA} ${WRKBUILD}/libsmapibsd.a ${PREFIX}/lib

.include <bsd.port.mk>
