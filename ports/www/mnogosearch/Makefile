# $RuOBSD: Makefile,v 1.3 2002/10/10 17:50:04 grange Exp $

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

CONFIGURE_STYLE=	gnu old
CONFIGURE_ARGS=		--sysconfdir=${SYSCONFDIR}/mnogosearch \
			--localstatedir=/var/mnogosearch

FLAVORS=	mysql postgresql
FLAVOR?=

.if ${FLAVOR:L:Mmysql}
LIB_DEPENDS+=	lib/mysql/mysqlclient.10:mysql-client-3.23.*:databases/mysql
CONFIGURE_ARGS+=--with-mysql=${LOCALBASE}
SQLSCRIPTS_DIR=	mysql
SQLSCRIPTS=	categories.txt crc-multi.txt crc.txt create.txt cross-crc.txt \
		cross.txt ispell.txt multi.txt servers.txt track.txt \
		url-raid.txt
.elif ${FLAVOR:L:Mpostgresql}
LIB_DEPENDS+=	pq.2::databases/postgresql
CONFIGURE_ARGS+=--with-pgsql=${LOCALBASE}
SQLSCRIPTS_DIR=	pgsql
SQLSCRIPTS=	categories.txt crc-multi.txt crc.txt create.txt cross-crc.txt \
		cross.txt grant.txt ispell.txt multi.txt news-extension.txt \
		server.txt to_use_with_tags.txt track.txt
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
.if defined(SQLSCRIPTS_DIR)
	${INSTALL_DATA_DIR} ${PREFIX}/share/mnogosearch
	cd ${WRKSRC}/create/${SQLSCRIPTS_DIR}; \
	${INSTALL_DATA} ${SQLSCRIPTS} ${PREFIX}/share/mnogosearch
.endif

.include <bsd.port.mk>
