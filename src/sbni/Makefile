#	$RuOBSD: Makefile,v 1.2 2001/01/17 07:21:44 form Exp $

SUBDIR=	sbniconfig

install-driver:
	@cd ${.CURDIR}/driver; sh install.sh

deinstall-driver:
	@cd ${.CURDIR}/driver; sh deinstall.sh

update-driver:
	@cd ${.CURDIR}/driver; sh install.sh

.include <bsd.subdir.mk>
