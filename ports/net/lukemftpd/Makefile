# $RuOBSD$
# $NetBSD: Makefile,v 1.5 2001/10/18 15:20:30 veego Exp $

COMMENT=		enhanced ftp server from NetBSD

DISTNAME=		lukemftpd-1.1
CATEGORIES=		net
MASTER_SITES=		ftp://ftp.netbsd.org/pub/NetBSD/misc/lukemftp/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MAINTAINER=		form@openbsd.ru

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		--with-skey
SEPARATE_BUILD=		concurrent

post-configure:
	@sed -e "s,ftpd,lukemftpd,g" < ${WRKSRC}/src/ftpd.conf.5 \
		> ${WRKBUILD}/src/lukemftpd.conf.5
	@sed -e "s,ftpd,lukemftpd,g" < ${WRKSRC}/src/ftpd.8 \
		> ${WRKBUILD}/src/lukemftpd.8

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/src/ftpd ${PREFIX}/libexec/lukemftpd
	${INSTALL_MAN} ${WRKBUILD}/src/lukemftpd.conf.5 ${PREFIX}/man/man5
	${INSTALL_MAN} ${WRKSRC}/src/ftpusers.5 ${PREFIX}/man/man5
	${INSTALL_MAN} ${WRKBUILD}/src/lukemftpd.8 ${PREFIX}/man/man8
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/lukemftpd
	${INSTALL_DATA} ${FILESDIR}/lukemftpd.conf.sample \
		${PREFIX}/share/examples/lukemftpd

.include <bsd.port.mk>
