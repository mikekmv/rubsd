# $RuOBSD: Makefile,v 1.5 2007/11/22 08:24:29 form Exp $

COMMENT=		"extensible ICQ client"
CATEGORIES=		net
PKGNAME=		${DISTNAME}p0

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
