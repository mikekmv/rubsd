# $RuOBSD: Makefile,v 1.3 2011/06/10 09:17:53 dinar Exp $

COMMENT=		"OSD plugin for licq"
CATEGORIES=		net x11
PKGNAME=		licq-osd-${VERSION}

WANTLIB=		ICE SM X11 Xext Xss c m pthread z

LIB_DEPENDS=		x11/xosd
RUN_DEPENDS=		net/licq/licq

USE_X11=		Yes
USE_GMAKE=		Yes
USE_LIBTOOL=		Yes

CONFIGURE_STYLE=	gnu

WRKDIST=		${WRKDIR}/${DISTNAME}/plugins/osd

CHECKSUM_FILE=		${.CURDIR}/../../distinfo

.include "${.CURDIR}/../../Makefile.inc"
.include <bsd.port.mk>
