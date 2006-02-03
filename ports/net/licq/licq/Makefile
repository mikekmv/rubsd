# $RuOBSD: Makefile,v 1.1.1.1 2006/01/30 00:46:39 mkb Exp $

COMMENT=		"Extensible ICQ client"

PKGNAME=		${DISTNAME}

WANTLIB=		c m pthread ssl stdc++

CONFIGURE_STYLE=	simple
CONFIGURE_ENV+=		"CPPFLAGS=-I${LOCALBASE}/include"
CONFIGURE_ARGS+=	--with-libiconv-prefix=${LOCALBASE} \
			--with-libintl-prefix=${LOCALBASE}

pre-configure:
	@perl -pi -e 's:/usr/share/licq:${PREFIX}/share/licq:' \
		${WRKSRC}/src/licq.conf.h

.include <bsd.port.mk>
