# $FreeBSD: ports/net/binkd/Makefile,v 1.12 2000/08/01 23:57:30 jedgar Exp $
# $RuOBSD: Makefile,v 1.2 2001/10/31 06:41:47 form Exp $

COMMENT=	"fidonet TCP/IP mailer"

DISTNAME=	binkd-0.9.4
DISTFILES=	binkd094.zip
CATEGORIES=	mail
MASTER_SITES=	ftp://happy.kiev.ua/pub/fidosoft/mailer/binkd/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=gnu

post-extract:
	@cp ${WRKSRC}/mkfls/unix/* ${WRKSRC}
	@chmod a+x ${WRKSRC}/configure

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/binkd ${PREFIX}/sbin
	${INSTALL_MAN} ${WRKSRC}/binkd.8 ${PREFIX}/man/man8
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/binkd
	${INSTALL_DATA} ${WRKSRC}/!README ${PREFIX}/share/doc/binkd/README
	${INSTALL_DATA} ${WRKSRC}/!README.FIX \
		${PREFIX}/share/doc/binkd/README.FIX
	${INSTALL_DATA} ${WRKSRC}/!SRIF.TXT ${PREFIX}/share/doc/binkd/SRIF.TXT
	${INSTALL_DATA} ${WRKSRC}/COPYING ${PREFIX}/share/doc/binkd
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/binkd
	${INSTALL_DATA} ${WRKSRC}/binkd.cfg \
		${PREFIX}/share/examples/binkd/binkd.cfg.sample

.include <bsd.port.mk>
