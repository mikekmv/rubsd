# $RuOBSD: Makefile,v 1.3 2003/08/15 19:51:51 grange Exp $

COMMENT=	"Bohlin's IrDA utilities"

DISTNAME=	birda-1.1
PKGNAME=	${DISTNAME}
CATEGORIES=	comms
MASTER_SITES=	ftp://ftp.augustsson.net/pub/netbsd/

MAINTAINER=	Pavel Korovin <pvk@oxygene.ru>

PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

FAKE_FLAGS=	PREFIX="${LOCALBASE}" MANDIR="${LOCALBASE}/man/cat" \
		LIBDIR="${LOCALBASE}/lib"

.include <bsd.port.mk>
