# $RuOBSD$

COMMENT=		"bpf traffic collector"

DISTNAME=		bpft-4.0_29-07-2002
PKGNAME=		bpft-4.0
EXTRACT_SUFX=		.tgz

CATEGORIES=		net

HOMEPAGE=		http://bpft.by.ru

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		http://bpft.by.ru

WRKDIST=		${WRKDIR}/bpft

COPTS=			-DHAVE_NETINET_IF_ETHER_H

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/trafd/trafd ${PREFIX}/sbin
	${INSTALL_MAN} ${WRKBUILD}/trafd/trafd.8 ${PREFIX}/man/man8
	${INSTALL_PROGRAM} ${WRKBUILD}/traflog/traflog ${PREFIX}/bin
	${INSTALL_MAN} ${WRKBUILD}/traflog/traflog.1 ${PREFIX}/man/man1
	${INSTALL_PROGRAM} ${WRKBUILD}/trafstat/trafstat ${PREFIX}/bin
	${INSTALL_MAN} ${WRKBUILD}/trafstat/trafstat.1 ${PREFIX}/man/man1
	${INSTALL_PROGRAM} ${WRKBUILD}/trafstatd/trafstatd ${PREFIX}/sbin
	${INSTALL_MAN} ${WRKBUILD}/trafstatd/trafstatd.8 ${PREFIX}/man/man8
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/bpft
	${INSTALL_DATA} ${WRKDIST}/traflog/traflog.format \
		${PREFIX}/share/examples/bpft

.include <bsd.port.mk>
