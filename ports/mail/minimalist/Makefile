# $RuOBSD$

COMMENT=		"fast and easy to setup mailing list manager"

DISTNAME=		minimalist-2.4
CATEGORIES=		mail

HOMEPAGE=		http://www.mml.org.ua

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/LIST/
EXTRACT_SUFX=		.tgz

NO_BUILD=		Yes

post-patch:
	@sed -e "s,/usr/local/sbin,${PREFIX}/libexec,g" \
	    -e "s,/usr/local/etc,${SYSCONFDIR},g" \
	    -e "s,minimalist\\.pl,minimalist,g" \
	    -e "s,/etc/aliases,/etc/mail/aliases,g" \
	    < ${WRKDIST}/docs/README > ${WRKBUILD}/README.OpenBSD

do-install:
	${INSTALL_SCRIPT} ${WRKDIST}/minimalist.pl ${PREFIX}/libexec/minimalist
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/minimalist
	${INSTALL_DATA} ${WRKBUILD}/README.OpenBSD \
	    ${PREFIX}/share/doc/minimalist/README
	${INSTALL_DATA} ${WRKDIST}/docs/virtual-domains.txt \
	    ${PREFIX}/share/doc/minimalist
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/minimalist
	${INSTALL_DATA} ${WRKDIST}/minimalist.conf-sample \
	    ${PREFIX}/share/examples/minimalist/minimalist.conf

.include <bsd.port.mk>
