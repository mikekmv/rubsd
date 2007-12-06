# $RuOBSD: Makefile,v 1.19 2006/08/06 16:19:53 form Exp $
# $FreeBSD: ports/irc/ircd-hybrid/Makefile,v 1.17 2001/02/01 10:43:34 phk Exp $

DISTNAME=		ircd-hybrid-7.2.3
CATEGORIES=		net
COMMENT=		"IRC server with many implemented options"
MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=ircd-hybrid/}
EXTRACT_SUFX=		.tgz

HOMEPAGE=		http://www.ircd-hybrid.org/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c crypto z

CONFIGURE_STYLE=	gnu
CONFIGURE_ENV=		CPPFLAGS="-I${LOCALBASE}/include" \
			LDFLAGS="-L${LOCALBASE}/lib"
CONFIGURE_ARGS=		--enable-kqueue --enable-openssl --enable-small-net \
			--enable-halfops --disable-shared-modules \
			--with-nicklen=13 --with-topiclen=255
CFLAGS+=		-DSYSCONFDIR='\"${SYSCONFDIR}\"' \
			-DIRCD_USER='\"_ircd\"' \
			-DHAVE_SNPRINTF

PSEUDO_FLAVORS?=	no_msgfilter
FLAVOR?=

IRCD_TOOLS=		encspeed mkkeypair mkpasswd untabify
IRCD_CONFS=		example.conf example.conf.quick simple.conf
IRCD_LANGS=		croatian danish dutch french german norwegian polish \
			russian spanish swedish

.if !${FLAVOR:Mno_msgfilter}
post-patch:
	@${ECHO_MSG} "===>  Applying msgfilter patch"
	@cd ${WRKDIST} && \
	    ${PATCH} -s ${PATCH_STRIP} < ${FILESDIR}/msgfilter.patch
.endif

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/src/ircd ${PREFIX}/libexec
	${INSTALL_PROGRAM} ${WRKBUILD}/servlink/servlink ${PREFIX}/libexec
.for f in ${IRCD_TOOLS}
	${INSTALL_PROGRAM} ${WRKBUILD}/tools/${f} ${PREFIX}/sbin/ircd-${f}
.endfor
	${INSTALL_MAN} ${WRKDIST}/doc/ircd.8 ${PREFIX}/man/man8
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/help/opers
	${INSTALL_DATA} ${WRKDIST}/contrib/help/opers/* \
	    ${WRKDIST}/help/opers/* ${PREFIX}/share/ircd/help/opers
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/help/users
	${INSTALL_DATA} ${WRKDIST}/contrib/help/users/* \
	    ${WRKDIST}/help/users/* \
	    ${PREFIX}/share/ircd/help/users
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/messages
.for f in ${IRCD_LANGS}
	${INSTALL_DATA} ${WRKDIST}/messages/ircd-${f}.lang \
	    ${PREFIX}/share/ircd/messages
.endfor
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/ircd
	${INSTALL_DATA} ${WRKDIST}/messages/ircd-standard.example.lang \
	    ${PREFIX}/share/examples/ircd
.for f in ${IRCD_CONFS}
	${INSTALL_DATA} ${WRKDIST}/etc/${f} ${PREFIX}/share/examples/ircd
.endfor

.include <bsd.port.mk>
