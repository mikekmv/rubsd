# $RuOBSD: Makefile,v 1.2 2002/04/24 04:58:03 form Exp $

COMMENT=		"some irc shit"

DISTNAME=		eggdrop1.6.8
PKGNAME=		eggdrop-1.6.8
CATEGORIES=		net

HOMEPAGE=		http://eggdrop.sourceforge.net

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${MASTER_SITE_SOURCEFORGE}
MASTER_SITE_SUBDIR=	eggdrop

LIB_DEPENDS=		tcl84::lang/tcl/8.4

CONFIGURE_STYLE=	autoconf

post-configure:
	@if cd ${WRKDIST}; then \
		sed -e "s,@prefix@,${PREFIX},g" \
			-e "s,@eggdropdir@,/var/eggdrop,g" \
			eggdrop.advanced.conf > eggdrop.advanced.conf.new; \
		sed -e "s,@prefix@,${PREFIX},g" \
			-e "s,@eggdropdir@,/var/eggdrop,g" \
			eggdrop.complete.conf > eggdrop.complete.conf.new; \
		sed -e "s,@prefix@,${PREFIX},g" \
			-e "s,@eggdropdir@,/var/eggdrop,g" \
			eggdrop.simple.conf > eggdrop.simple.conf.new; \
		sed -e "s,@prefix@,${PREFIX},g" \
			-e "s,@eggdropdir@,/var/eggdrop,g" \
			src/main.c > src/eggdrop.c; \
		sed -e "s,@prefix@,${PREFIX},g" \
			-e "s,@eggdropdir@,/var/eggdrop,g" \
			src/eggdrop.h > src/eggdrop_env.h; \
	fi
	@cd ${WRKBUILD} && ${MAKE} config

do-install:
	@cd ${WRKBUILD} && ${MAKE} install DEST=${PREFIX}/eggdrop
	${INSTALL_DATA} ${WRKDIST}/eggdrop.advanced.conf.new \
		${PREFIX}/eggdrop/eggdrop.advanced.conf
	${INSTALL_DATA} ${WRKDIST}/eggdrop.complete.conf.new \
		${PREFIX}/eggdrop/eggdrop.complete.conf
	${INSTALL_DATA} ${WRKDIST}/eggdrop.simple.conf.new \
		${PREFIX}/eggdrop/eggdrop.simple.conf

.include <bsd.port.mk>
