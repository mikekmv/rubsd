# $RuOBSD: Makefile,v 1.1.1.1 2002/10/14 07:09:09 grange Exp $

COMMENT=		"GNOME jabber client"

DISTNAME=		gabber-0.8.7
CATEGORIES=		net x11/gnome

HOMEPAGE=		http://gabber.sourceforge.net/

MAINTAINER=		Alexander Yurchenko <grange@rt.mipt.ru>

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

MASTER_SITES=		${MASTER_SITE_SOURCEFORGE:=gabber/}

LIB_DEPENDS=		sigc.0::devel/libsigc++ gal::devel/gal

USE_X11=		Yes

CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		--enable-ipv6

FLAVORS=		xmms
FLAVOR?=

.if ${FLAVOR:L:Mxmms}
LIB_DEPENDS+=		xmms.3::audio/xmms
RUN_DEPENDS+=		:xmms:audio/xmms
.else
CONFIGURE_ARGS+=	--disable-xmms
.endif

.include <bsd.port.mk>
