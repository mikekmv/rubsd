# $RuOBSD$

COMMENT=		"full-featured web search engine"

DISTNAME=		mnogosearch-3.1.20
CATEGORIES=		www

HOMEPAGE=		http://www.mnogosearch.org/

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		http://www.mnogosearch.org/Download/

MNOGOSEARCHDIR?=	/var/mnogosearch

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS+=	--sysconfdir=${SYSCONFDIR}/mnogosearch --localstatedir=${MNOGOSEARCHDIR}

FLAVORS=	mysql postgresql
FLAVOR?=

.if ${FLAVOR:L:Mmysql}
LIB_DEPENDS+=	lib/mysql/mysqlclient.10:mysql-client-3.23.*:databases/mysql
CONFIGURE_ARGS+=--with-mysql=${LOCALBASE}
.elif ${FLAVOR:L:Mpostgresql}
LIB_DEPENDS+=	pq.2::databases/postgresql
CONFIGURE_ARGS+=--with-pgsql=${LOCALBASE}
.else
CONFIGURE_ARGS+=--with-built-in
.endif

post-install:
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/mnogosearch
	${INSTALL_DATA} ${WRKSRC}/etc/indexer.conf-dist \
	    ${PREFIX}/share/examples/mnogosearch
	${INSTALL_DATA} ${WRKSRC}/etc/search.htm-dist \
	    ${PREFIX}/share/examples/mnogosearch
	${INSTALL_DATA} ${WRKSRC}/etc/spelld.conf-dist \
	    ${PREFIX}/share/examples/mnogosearch

.include <bsd.port.mk>
