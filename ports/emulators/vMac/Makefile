# $RuOBSD$
# $FreeBSD: ports/emulators/vMac/Makefile,v 1.5 2000/10/08 10:20:05 asami Exp $

COMMENT=		"MacPlus machine emulator! Runs MacOS versions up to 7.5.5"

DISTNAME=		vMac-0.1.9.3
DISTFILES=		vMac-0.1.9.3-src.tgz
CATEGORIES=		emulators
MASTER_SITES=		http://www.vmac.org/download/XWindows/

HOMEPAGE=		http://www.vmac.org

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MAINTAINER=		Oleg Safiullin <form@openbsd.org>

USE_X11=		Yes
USE_GMAKE=	 	Yes

CONFIGURE_STYLE=	gnu

post-extract:
	@gunzip -c ${WRKDIST}/vmac.1.gz > ${WRKDIST}/vMac.1

pre-patch:
	@tr -d '\015' < ${WRKDIST}/gemulator.c > ${WRKDIST}/gemulator.new && \
		mv -f ${WRKDIST}/gemulator.new ${WRKDIST}/gemulator.c

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/vMac ${PREFIX}/bin
	${INSTALL_MAN} ${WRKDIST}/vMac.1 ${PREFIX}/man/man1
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/vMac
	${INSTALL_DATA} ${WRKDIST}/CREDITS ${PREFIX}/share/doc/vMac
	${INSTALL_DATA} ${WRKDIST}/INSTALL  ${PREFIX}/share/doc/vMac
	${INSTALL_DATA} ${WRKDIST}/README ${PREFIX}/share/doc/vMac
	${INSTALL_DATA} ${WRKDIST}/README.rom ${PREFIX}/share/doc/vMac

.include <bsd.port.mk>
