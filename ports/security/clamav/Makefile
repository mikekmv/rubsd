# $RuOBSD: Makefile,v 1.4 2004/07/01 06:30:14 form Exp $

COMMENT=		"free virus scanner"
COMMENT-milter=		"sendmail/libmilter interface for clamav"

DISTNAME=		clamav-${VERSION}
PKGNAME-milter=		clamav-milter-${VERSION}
VERSION=		0.80

CATEGORIES=		security

HOMEPAGE=		http://www.clamav.net/

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=clamav/}

LIB_DEPENDS=		gmp::devel/gmp

RUN_DEPENDS=		lha::archivers/lha \
			unarj::archivers/unarj \
			unrar::archivers/unrar \
			zoo::archivers/zoo \
			arc::archivers/arc \
			unzip::archivers/unzip \
			bzip2::archivers/bzip2

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		${CONFIGURE_SHARED} --with-dbdir=/var/clamav/db \
			--with-user=_clamav --with-group=_clamav \
			--disable-clamav --disable-cr --enable-bigstack \
			--disable-clamuko
CONFIGURE_ENV=		CPPFLAGS="-I${LOCALBASE}/include" \
			LDFLAGS="-pthread -L${LOCALBASE}/lib"

.if exists(/usr/lib/libmilter.a) && exists(/usr/include/libmilter/mfapi.h)
CONFIGURE_ARGS+=	--enable-milter
MULTI_PACKAGES=		-milter
.else
CONFIGURE_ARGS+=	--disable-milter
.endif

# XXX
.if defined(PACKAGING)
. if defined(SUBPACKAGE) && ${SUBPACKAGE} == "-milter"
#RUN_DEPENDS=		::security/clamav
RUN_DEPENDS=		::${.CURDIR}
. endif
.endif

DOCS=			clamav-mirror-howto.pdf clamdoc.pdf signatures.pdf
DOCS_DIRS=		html

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/clamav
	${INSTALL_DATA} ${WRKDIST}/etc/clamd.conf \
	    ${PREFIX}/share/examples/clamav
	${INSTALL_DATA} ${WRKDIST}/etc/freshclam.conf \
	    ${PREFIX}/share/examples/clamav
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/clamav/db
	${INSTALL_DATA} ${WRKDIST}/database/daily.cvd \
	    ${PREFIX}/share/examples/clamav/db
	${INSTALL_DATA} ${WRKDIST}/database/main.cvd \
	    ${PREFIX}/share/examples/clamav/db
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/clamav
.for d in ${DOCS}
	${INSTALL_DATA} ${WRKDIST}/docs/${d} ${PREFIX}/share/doc/clamav
.endfor
.for d in ${DOCS_DIRS}
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/clamav/${d}
	${INSTALL_DATA} ${WRKDIST}/docs/${d}/*.* \
	    ${PREFIX}/share/doc/clamav/${d}
.endfor

.include <bsd.port.mk>
