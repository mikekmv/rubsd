# $RuOBSD$

COMMENT=		"Extensible ICQ client"

PKGNAME=		${DISTNAME}

WANTLIB=		m c z pthread ssl

CONFIGURE_STYLE=	simple
CONFIGURE_ENV+=		"CPPFLAGS=-I${LOCALBASE}/include"
CONFIGURE_ARGS+=	--with-libiconv-prefix=${LOCALBASE} \
			--with-libintl-prefix=${LOCALBASE}

pre-configure:
	@perl -pi -e 's:/usr/share/licq:${PREFIX}/share/licq:' \
		${WRKSRC}/src/licq.conf.h

.include <bsd.port.mk>
