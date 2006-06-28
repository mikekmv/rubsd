# $RuOBSD: Makefile,v 1.2 2006/06/22 16:57:41 form Exp $

COMMENT=		"fast and easy to setup mailing list manager"

DISTNAME=		minimalist-2.5.3
CATEGORIES=		mail

HOMEPAGE=		http://www.mml.org.ua

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${HOMEPAGE}/LIST/

NO_BUILD=		Yes

FLAVORS=		pkoi
FLAVOR?=

post-patch:
	@sed -e "s|/usr/local/etc|${SYSCONFDIR}|g" \
	    < ${WRKDIST}/minimalist.pl > ${WRKBUILD}/minimalist
	@sed -e "s|/usr/local/etc|${SYSCONFDIR}|g" \
	    -e "s|/usr/local/sbin|${PREFIX}/libexec|g" \
	    -e "s|/etc/aliases|/etc/mail/aliases|g" \
	    < ${WRKDIST}/docs/README > ${WRKBUILD}/README
.if ${FLAVOR:L:Mpkoi}
	@cd ${WRKDIST} && patch -sp0 < ${FILESDIR}/ru-plain-koi8-r.patch && \
	    rm -f minimalist.orig
.endif

do-install:
	${INSTALL_SCRIPT} ${WRKBUILD}/minimalist ${PREFIX}/libexec
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/minimalist
	${INSTALL_DATA} ${WRKBUILD}/README ${PREFIX}/share/doc/minimalist
	${INSTALL_DATA} ${WRKDIST}/docs/virtual-domains.txt \
	    ${PREFIX}/share/doc/minimalist
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/minimalist
	${INSTALL_DATA} ${WRKDIST}/minimalist.conf-sample \
	    ${PREFIX}/share/examples/minimalist

.include <bsd.port.mk>
