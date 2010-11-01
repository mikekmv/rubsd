# $RuOBSD  $

COMMENT=	DEC VMS password hashing

MODULES=	cpan
DISTNAME=	Authen-DecHpwd-2.005

CATEGORIES=	security

# Artistic/GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	modbuild

BUILD_DEPENDS=	::devel/p5-Scalar-String \
		::devel/p5-Data-Integer
		
RUN_DEPENDS=	${BUILD_DEPENDS}

.include <bsd.port.mk>
