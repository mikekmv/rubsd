# $RuOBSD: Makefile,v 1.3 2010/11/02 06:26:37 dinar Exp $

PKGNAME=	ikiwiki-${VERSION}
VERSION=	3.20101023
CATEGORIES=	www
MASTER_SITES=	${MASTER_SITE_DEBIAN_POOL} \
		http://kitenet.net/~joey/debian/unstable/ \
		ftp://ftp.openbsd.ru/pub/OpenBSD/local/distfiles/
		
DISTNAME=	ikiwiki_${VERSION}

MAINTAINER=	dinar@yantel.ru
COMMENT=	A wiki compiler

BUILD_DEPENDS=	::www/p5-HTML-Parser \
		::devel/p5-Locale-gettext \
		::www/p5-CGI-FormBuilder \
		::www/p5-CGI-Session \
		::devel/p5-Time-TimeDate \
		::www/p5-HTML-Scrubber \
		::www/p5-HTML-Template \
		::mail/p5-Mail-Sendmail \
		::textproc/p5-Text-Markdown \
		::www/p5-URI \
		::textproc/p5-XML-Simple
RUN_DEPENDS=	::devel/p5-File-MimeInfo \
		::net/p5-RPC-XML \
		::www/p5-XML-Feed \
		::security/p5-Authen-Passphrase \
		::devel/p5-File-chdir \
		::devel/p5-File-ReadBackwards \
		::devel/cvsps \
		::www/p5-HTML-Tree \
		::lang/python/2.6



WRKDIST=	${WRKDIR}/ikiwiki

# GPL
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

CONFIGURE_STYLE=	perl
USE_GMAKE=	yes
USE_GETTEXT=	yes

FLAVORS =		search
FLAVOR ?=

.if ${FLAVOR:L:Msearch}
RUN_DEPENDS+=	::security/p5-Digest-SHA1 \
		::databases/p5-Search-Xapian \
		::www/xapian-omega
.endif



.include <bsd.port.mk>
