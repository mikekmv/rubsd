# $RuOBSD$

COMMENT=		"file alteration monito"

DISTNAME=		fam-2.6.9
CATEGORIES=		sysutils devel

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

HOMEPAGE=		http://oss.sgi.com/projects/fam/

MASTER_SITES=		ftp://oss.sgi.com/projects/fam/download/ \
			ftp://ftp.tuwien.ac.at/opsys/linux/gentoo/distfiles/ \
			http://gd.tuwien.ac.at/opsys/linux/gentoo/distfiles/

AUTOCONF_VERSION=	2.13
CONFIGURE_STYLE=	autoconf
USE_LIBTOOL=		Yes
USE_GMAKE=		Yes

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/fam/fam ${PREFIX}/bin
	${INSTALL_DATA} ${WRKBUILD}/include/fam.h ${PREFIX}/include
	${INSTALL_DATA} ${WRKBUILD}/libfam/.libs/libfam.a ${PREFIX}/lib
	${INSTALL_DATA} ${WRKBUILD}/libfam/.libs/libfam.so.0.0 ${PREFIX}/lib
	${INSTALL_DATA_DIR} ${PREFIX}/lib/fam
	${INSTALL_SCRIPT} ${WRKBUILD}/util/editconf/editconf.perl \
	    ${PREFIX}/lib/fam
	${INSTALL_MAN} ${WRKBUILD}/man/fam.1m ${PREFIX}/man/man1/fam.1
	${INSTALL_MAN} ${WRKDIST}/man/fam.3x ${PREFIX}/man/man3/fam.3
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/fam
	${INSTALL_DATA} ${WRKBUILD}/fam/fam.conf ${PREFIX}/share/examples/fam

.include <bsd.port.mk>
