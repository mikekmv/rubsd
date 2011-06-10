# $RuOBSD: Makefile,v 1.2 2011/06/10 09:17:53 dinar Exp $

COMMENT=		"PDP-11 emulator"

DISTNAME=		p11-2.10i
EXTRACT_SUFX=		.tar.bz2
CATEGORIES=		emulators

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c

LIB_DEPENDS=		devel/libbegemot

MASTER_SITES=		http://people.freebsd.org/~harti/p11/

USE_GMAKE=		Yes
CONFIGURE_STYLE=	gnu
CONFIGURE_ARGS=		--with-begemot=${LOCALBASE}

post-install:
	chmod ${BINMODE} ${PREFIX}/libexec/p11/epp_bpf
	chmod ${BINMODE} ${PREFIX}/libexec/p11/epp_tun

.include <bsd.port.mk>
