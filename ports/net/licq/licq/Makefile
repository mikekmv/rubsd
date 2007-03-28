# $RuOBSD: Makefile,v 1.2 2006/02/03 17:46:03 mkb Exp $

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

post-install:
	${INSTALL_SCRIPT} ${FILESDIR}/viewurl-firefox.sh ${PREFIX}/bin

.include <bsd.port.mk>
