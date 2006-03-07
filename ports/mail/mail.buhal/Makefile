# $RuOBSD$

COMMENT=		"local maildir mail delivery agent"

DISTNAME=		mail.buhal-1.0
CATEGORIES=		mail

HOMEPAGE=		http://pdp-11.org.ru/~form/openbsd

# BSD
PERMIT_PACKAGE_CDROM=	Yes
PERMIT_PACKAGE_FTP=	Yes
PERMIT_DISTFILES_CDROM=	Yes
PERMIT_DISTFILES_FTP=	Yes

WANTLIB=		c

MASTER_SITES=		${HOMEPAGE}/files/

PREFIX?=		/usr

do-install:
	${INSTALL_PROGRAM_DIR} ${PREFIX}/libexec
	${INSTALL_PROGRAM} ${WRKBUILD}/mail.buhal/mail.buhal \
	    ${PREFIX}/libexec
	${INSTALL_MAN_DIR} ${PREFIX}/share/man/cat8
	${INSTALL_MAN} ${WRKBUILD}/mail.buhal/mail.buhal.cat8 \
	    ${PREFIX}/share/man/cat8/mail.buhal.0

.include <bsd.port.mk>
