# $RuOBSD: Makefile,v 1.7 2007/11/23 04:37:02 form Exp $

COMMENT=		"extensible ICQ client"
CATEGORIES=		net
PKGNAME=		${DISTNAME}p2

WANTLIB=		c crypto m pthread ssl stdc++

MODULES=		devel/gettext

CONFIGURE_STYLE=	gnu
CONFIGURE_ENV=		"CPPFLAGS=-I${LOCALBASE}/include"
CONFIGURE_ARGS=		--with-libiconv-prefix=${LOCALBASE} \
			--with-libintl-prefix=${LOCALBASE}

CHECKSUM_FILE=		${.CURDIR}/../distinfo

pre-configure:
	@perl -pi -e 's:/usr/share/licq:${PREFIX}/share/licq:' \
		${WRKSRC}/src/licq.conf.h

.include <bsd.port.mk>
