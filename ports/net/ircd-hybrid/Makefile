# $RuOBSD: Makefile,v 1.5 2004/03/24 10:58:28 form Exp $
# $FreeBSD: ports/irc/ircd-hybrid/Makefile,v 1.17 2001/02/01 10:43:34 phk Exp $

DISTNAME=		ircd-hybrid-7.0.1
CATEGORIES=		net
COMMENT=		"IRC server with many implemented options"
MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=ircd-hybrid/}
EXTRACT_SUFX=		.tgz

HOMEPAGE=		http://ircd-hybrid.sourceforge.net/

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	gnu dest
CONFIGURE_ARGS=		--enable-kqueue --enable-openssl --enable-zlib \
			--with-nicklen=13 --with-topiclen=255 \
			--enable-small-net --disable-shared-modules

IRCD_TOOLS=		convertconf convertilines convertklines encspeed \
			mkkeypair mkpasswd untabify viconf
IRCD_CONFS=		simple.conf dline.conf example.conf \
			example.efnet.conf ircd.motd kline.conf

do-install:
	${INSTALL_PROGRAM} ${WRKBUILD}/src/ircd ${PREFIX}/libexec
	${INSTALL_PROGRAM} ${WRKBUILD}/servlink/servlink ${PREFIX}/libexec
.for f in ${IRCD_TOOLS}
	${INSTALL_PROGRAM} ${WRKBUILD}/tools/${f} ${PREFIX}/sbin/ircd.${f}
.endfor
	${INSTALL_MAN} ${WRKDIST}/doc/ircd.8 ${PREFIX}/man/man8
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/help/opers
	${INSTALL_DATA} ${WRKDIST}/help/opers/* ${PREFIX}/share/ircd/help/opers
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/help/users
	${INSTALL_DATA} ${WRKDIST}/help/users/* ${PREFIX}/share/ircd/help/users
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/messages/ayb/LC_MESSAGES
	${INSTALL_DATA} ${WRKBUILD}/messages/ayb.mo \
	    ${PREFIX}/share/ircd/messages/ayb/LC_MESSAGES/ircd-hybrid.mo
	${INSTALL_DATA_DIR} ${PREFIX}/share/ircd/messages/custom/LC_MESSAGES
	${INSTALL_DATA} ${WRKBUILD}/messages/custom.mo \
	    ${PREFIX}/share/ircd/messages/ayb/LC_MESSAGES
	${INSTALL_DATA_DIR} ${PREFIX}/share/examples/ircd
.for f in ${IRCD_CONFS}
	${INSTALL_DATA} ${WRKDIST}/doc/${f} ${PREFIX}/share/examples/ircd
.endfor

.include <bsd.port.mk>
