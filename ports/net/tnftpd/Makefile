# $RuOBSD: Makefile,v 1.1.1.1 2005/11/21 23:29:17 form Exp $
# $NetBSD: Makefile,v 1.5 2001/10/18 15:20:30 veego Exp $

COMMENT=		enhanced FTP server from NetBSD

DISTNAME=		tnftpd-20040810
CATEGORIES=		net
MASTER_SITES=		ftp://ftp.netbsd.org/pub/NetBSD/misc/tnftp/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CFLAGS+=		-DSUPPORT_UTMP

CONFIGURE_STYLE=	gnu
SEPARATE_BUILD=		concurrent

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/src/tnftpd ${PREFIX}/libexec
	${INSTALL_MAN} ${WRKDIST}/src/ftpd.conf.5 \
		${PREFIX}/man/man5/tnftpd.conf.5
	${INSTALL_MAN} ${WRKDIST}/src/ftpusers.5 \
		${PREFIX}/man/man5/tnftpusers.5
	@ln ${PREFIX}/man/man5/tnftpusers.5 ${PREFIX}/man/man5/tnftpchroot.5
	${INSTALL_MAN} ${WRKDIST}/src/ftpd.8 ${PREFIX}/man/man8/tnftpd.8
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/tnftpd
	${INSTALL_DATA} ${WRKDIST}/examples/ftpd.conf \
		${PREFIX}/share/examples/tnftpd/tnftpd.conf.sample
	${INSTALL_DATA} ${WRKDIST}/examples/ftpusers \
		${PREFIX}/share/examples/tnftpd/tnftpusers.sample

.include <bsd.port.mk>
