# $OpenBSD$

COMMENT=		"DrWeb Virus Scanner"
VERSION=		4.32.1_1
DISTNAME=		drweb-${VERSION}-openbsd34
PKGNAME=		drweb-${VERSION}
CATEGORIES=		security

HOMEPAGE=		http://www.sald.com/

DISTFILES=		${DISTNAME}.tar.gz

MASTER_SITES=		http://download.us.drweb.com/files/unix/OpenBSD/34/

PERMIT_PACKAGE_CDROM=	No
PERMIT_PACKAGE_FTP=	No
PERMIT_DISTFILES_CDROM=	No
PERMIT_DISTFILES_FTP=	No

NO_BUILD=		Yes

# refer to and respect /usr/ports/infrastucture/db/user.list
DRWEBUSER=		_drweb
DRWEBGROUP=		_drweb
DRWEBID=		533

SUBST_VARS=		DRWEBUSER DRWEBGROUP DRWEBID

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/drweb
	${INSTALL_DATA_DIR} ${PREFIX}/drweb/bin
	${INSTALL_DATA_DIR} ${PREFIX}/drweb/sbin
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/drweb
	${INSTALL_DATA} ${WRKSRC}/usr/local/drweb/doc/* ${PREFIX}/share/doc/drweb
	${INSTALL_DATA} ${WRKSRC}/usr/local/drweb/update/readme* ${PREFIX}/share/doc/drweb
	${INSTALL_DATA} ${WRKSRC}/usr/local/drweb/getkey* ${PREFIX}/share/doc/drweb
	${INSTALL_SCRIPT} ${WRKSRC}/usr/local/drweb/update/update.pl ${PREFIX}/drweb/bin
	${INSTALL_PROGRAM} ${WRKSRC}/usr/local/drweb/drweb ${PREFIX}/drweb/bin
	${INSTALL_PROGRAM} ${WRKSRC}/usr/local/drweb/drweb.static ${PREFIX}/drweb/bin
	${INSTALL_PROGRAM} ${WRKSRC}/usr/local/drweb/drwebd ${PREFIX}/drweb/sbin
	${INSTALL_PROGRAM} ${WRKSRC}/usr/local/drweb/drwebd.static ${PREFIX}/drweb/sbin
	

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/drweb
	${INSTALL_DATA} ${WRKSRC}/usr/local/etc/drweb/*.ini ${PREFIX}/share/examples/drweb
	${INSTALL_DATA} ${WRKSRC}/usr/local/etc/rc.d/* ${PREFIX}/share/examples/drweb
	${INSTALL_DATA_DIR} ${PREFIX}/share/drweb/bases
	${INSTALL_DATA} ${WRKSRC}/var/drweb/bases/* ${PREFIX}/share/drweb/bases
	${INSTALL_DATA_DIR} ${PREFIX}/share/drweb/lib
	${INSTALL_DATA} ${WRKSRC}/usr/local/drweb/lib/* ${PREFIX}/share/drweb/lib
	${INSTALL_DATA_DIR} ${PREFIX}/share/drweb/infected
	${INSTALL_DATA_DIR} ${PREFIX}/share/drweb/log
	${INSTALL_DATA_DIR} ${PREFIX}/share/drweb/run
	${INSTALL_DATA_DIR} ${PREFIX}/share/drweb/spool


NO_REGRESS=     Yes
.include <bsd.port.mk>
