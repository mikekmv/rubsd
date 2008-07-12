# $RuOBSD: Makefile$

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

MASTER_SITES=		${HOMEPAGE}/files/misc/

post-build:
	@sed -e "s,/usr/libexec,${PREFIX}/libexec,g" \
	    ${WRKBUILD}/mail.buhal/mail.buhal.cat8 \
	    > ${WRKBUILD}/mail.buhal/mail.buhal.new.cat8

do-install:
	${INSTALL_PROGRAM_DIR} ${PREFIX}/libexec
	${INSTALL_PROGRAM} ${WRKBUILD}/mail.buhal/mail.buhal \
	    ${PREFIX}/libexec
	${INSTALL_MAN} ${WRKBUILD}/mail.buhal/mail.buhal.new.cat8 \
	    ${PREFIX}/man/cat8/mail.buhal.0

.include <bsd.port.mk>
