#	$RuOBSD: Makefile,v 1.4 2001/11/15 03:54:27 form Exp $

OPSYS!=	uname -s
OPVER!=	uname -r

.if exists(Makefile.${OPSYS})
.include "Makefile.${OPSYS}"
.include <bsd.prog.mk>
.else
${.TARGET}:
	@echo "${OPSYS} platform not supported"
.endif
