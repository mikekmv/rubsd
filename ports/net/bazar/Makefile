# $RuOBSD: Makefile,v 1.3 2000/06/23 06:07:27 form Exp $

PKGNAME=	bazar-1.0
COMMENT=	"talk(1) replacement with cp1251 <> koi8 recoding support"
CATEGORIES=	net
DISTFILES=
NO_CHECKSUM=	Yes

MAINTAINER=	form@openbsd.ru

PERMIT_PACKAGE_CDROM=   Yes
PERMIT_PACKAGE_FTP=     Yes
PERMIT_DISTFILES_CDROM= Yes
PERMIT_DISTFILES_FTP=   Yes

WRKDIST=	${WRKDIR}/${PKGNAME}

FAKE_FLAGS=	PREFIX=${PREFIX}

do-configure:
	@cd ${WRKSRC} && ${MAKE} depend

post-extract:
	@mkdir -p ${WRKSRC}
	@cp -RP /usr/src/usr.bin/talk/* ${WRKSRC}
	@rm -f ${WRKSRC}/obj ${WRKSRC}/obj.${MACHINE_ARCH}
	@rm -fr ${WRKSRC}/CVS
	@mv ${WRKSRC}/talk.1 ${WRKSRC}/bazar.1
	@cp ${FILESDIR}/charset.c ${FILESDIR}/charset.h ${WRKSRC}

.include <bsd.port.mk>
