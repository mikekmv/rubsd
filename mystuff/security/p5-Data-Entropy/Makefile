# $RuOBSD  $

COMMENT=	Entropy (randomness) management

MODULES=	cpan
DISTNAME=	Data-Entropy-0.006

CATEGORIES=	security

# Artistic/GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

BUILD_DEPENDS=	::security/p5-Crypt-Rijndael \
		::www/p5-libwww \
		::devel/p5-Params-Classify \
		::devel/p5-Data-Float
		
RUN_DEPENDS=${BUILD_DEPENDS}			

CONFIGURE_STYLE=	modbuild


.include <bsd.port.mk>
