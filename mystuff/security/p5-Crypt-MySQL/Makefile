# $RuOBSD$

COMMENT=	extension to compare MySQL passwords without libmysqlclient

MODULES=	cpan
DISTNAME=	Crypt-MySQL-0.04


CATEGORIES=	security

# Artistic/GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	modbuild

RUN_DEPENDS=		::security/p5-Digest-SHA1

.include <bsd.port.mk>
