# $RuOBSD: Makefile,v 1.3 2011/06/10 09:17:54 dinar Exp $

COMMENT=	perl interface to hashed passwords/passphrases as objects

MODULES=	cpan
DISTNAME=	Authen-Passphrase-0.007

CATEGORIES=	security

# Artistic/GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	modbuild

BUILD_DEPENDS=	security/p5-Authen-DecHpwd \
		security/p5-Crypt-Blowfish \
		security/p5-Crypt-DES \
		security/p5-Crypt-Eksblowfish \
		security/p5-Crypt-MySQL \
		security/p5-Crypt-PasswdMD5 \
		security/p5-Crypt-UnixCrypt_XS \
		security/p5-Data-Entropy \
		security/p5-Digest-MD4 \
		security/p5-Digest-SHA1 \
		devel/p5-Module-Runtime \
		devel/p5-Params-Classify
RUN_DEPENDS=	${BUILD_DEPENDS}

.include <bsd.port.mk>
