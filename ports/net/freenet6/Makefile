# $RuOBSD: Makefile,v 1.1.1.1 2003/08/11 11:08:49 form Exp $

COMMENT=		"Freenet6 Tunnel Setup Protocol Client"

DISTNAME=		freenet6-0.9.8
CATEGORIES=		net

HOMEPAGE=		http://www.freenet6.net

PERMIT_PACKAGE_CDROM=	"Redistribution not allowed"
PERMIT_PACKAGE_FTP=	"Redistribution not allowed"
PERMIT_DISTFILES_CDROM=	"Redistribution not allowed"
PERMIT_DISTFILES_FTP=	"Redistribution not allowed"

EXTRACT_SUFX=		.tgz

FETCH_MANUALLY=		"May not be automatically fetched due to licensing" \
			"restrictions. You MUST manually fetch the version" \
			"after reading and agreeing to the license at:" \
			"${HOMEPAGE}/cgi-bin/download.cgi?fn=${DISTFILES}" \
			"Once it has been downloaded, move it to" \
			"${DISTDIR} and then restart this build." \
			"You also can register to use this service at" \
			"${HOMEPAGE}/cgi-bin/new_account.pl"

CFLAGS+=		-DSYSCONFDIR=\"${SYSCONFDIR}\" \
			-DLOGDIR=\"/var/log\" \
			-DSCRIPTDIR=\"${PREFIX}/libexec/tspc\"

ALL_TARGET=		all
MAKE_FLAGS=		target=openbsd tsp_dir=${SYSCONFDIR}/tspc

NO_REGRESS=		Yes

post-patch:
	@sed -e "s,/install/dir/bin/tspc\.conf,${SYSCONFDIR}/tspc.conf,g" \
	    < ${WRKDIST}/man/man5/tspc.conf.5 \
	    > ${WRKBUILD}/man/man5/tspc.conf.5.NEW
	@sed -e "s,/install/dir/bin/tspc\.conf,${SYSCONFDIR}/tspc.conf,g" \
	    < ${WRKDIST}/man/man8/tspc.8 \
	    > ${WRKBUILD}/man/man8/tspc.8.NEW

do-install:
	${INSTALL_PROGRAM_DIR} ${PREFIX}/libexec/tspc
	${INSTALL_PROGRAM} ${WRKBUILD}/bin/tspc ${PREFIX}/libexec/tspc
	${INSTALL_SCRIPT} ${WRKBUILD}/template/openbsd.sh \
	    ${PREFIX}/libexec/tspc
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/freenet6
	${INSTALL_MAN} ${WRKBUILD}/man/man5/tspc.conf.5.NEW \
	    ${PREFIX}/man/man5/tspc.conf.5
	${INSTALL_MAN} ${WRKBUILD}/man/man8/tspc.8.NEW \
	    ${PREFIX}/man/man8/tspc.8
	${INSTALL_DATA} ${WRKBUILD}/bin/tspc.conf \
	    ${PREFIX}/share/examples/freenet6

.include <bsd.port.mk>
