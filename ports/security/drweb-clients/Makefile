# $OpenBSD$

COMMENT=		"DrWeb Milter interface"
VERSION=		4.32
DISTNAME=		drweb-clients-${VERSION}-sources
PKGNAME=		drweb-clients-${VERSION}
CATEGORIES=		security

HOMEPAGE=		http://www.sald.com/

DISTFILES=		${DISTNAME}.tar.gz

MASTER_SITES=		http://download.us.drweb.com/files/unix/Clients/

PERMIT_PACKAGE_CDROM=	No
PERMIT_PACKAGE_FTP=	No
PERMIT_DISTFILES_CDROM=	No
PERMIT_DISTFILES_FTP=	No

# refer to and respect /usr/ports/infrastucture/db/user.list
DRWEBUSER=		_drweb
DRWEBGROUP=		_drweb
DRWEBID=		533

SUBST_VARS=		DRWEBUSER DRWEBGROUP DRWEBID

CONFIGURE_STYLE=	simple

pre-configure:
	cp -R /usr/lib/libmilter* ${WRKSRC}/lib
	cp -R /usr/include/libmilter ${WRKSRC}/include

do-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/drweb-clients
	cp -R ${WRKSRC}/doc/* ${PREFIX}/share/doc/drweb-clients
	${INSTALL_DATA_DIR} ${PREFIX}/drweb
	${INSTALL_DATA_DIR} ${PREFIX}/drweb/bin
	${INSTALL_PROGRAM} ${WRKSRC}/drweb-smf ${PREFIX}/drweb/bin
	${INSTALL_PROGRAM} ${WRKSRC}/drwebdc ${PREFIX}/drweb/bin
	

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/drweb-clients
	${INSTALL_DATA} ${WRKSRC}/etc/bsd/*.conf ${PREFIX}/share/examples/drweb-clients
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/drweb-clients/templates
	cp -R ${WRKSRC}/etc/templates/* ${PREFIX}/share/examples/drweb-clients/templates

NO_REGRESS=     Yes
.include <bsd.port.mk>
