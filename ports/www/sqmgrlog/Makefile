# $RuOBSD$

DISTNAME=	
CATEGORIES=	www
NEED_VERSION=	1.301
DISTNAME=	sqmgrlog-2.10

MASTER_SITES=	http://www.ineparnet.com.br/orso/

MAINTAINER=	form@openbsd.ru

PERMIT_PACKAGE_CDROM=   Yes
PERMIT_PACKAGE_FTP=     Yes
PERMIT_DISTFILES_CDROM= Yes
PERMIT_DISTFILES_FTP=   Yes

FLAVORS=	ru ru_win
FLAVOR?=

.if ${FLAVOR:L} == "ru"
CONFIGURE_LANG=	Russian_koi8
.elif ${FLAVOR:L} == "ru_win"
CONFIGURE_LANG=	Russian_windows1251
.else
FLAVOR=
CONFIGURE_LANG=	English
.endif

CONFIGURE_STYLE=gnu
CONFIGURE_ARGS=	--enable-config=/etc/squid --enable-prefix=${PREFIX}/sbin \
		--enable-language=${CONFIGURE_LANG}

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/sqmgrlog ${PREFIX}/sbin
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/sqmgrlog
	${INSTALL_DATA} ${WRKBUILD}/sqmgrlog.conf \
		${PREFIX}/share/examples/sqmgrlog

.include <bsd.port.mk>
