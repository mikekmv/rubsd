#	$RuOBSD$

OPSYS!=	uname -s

.if exists(Makefile.${OPSYS})
.include "Makefile.${OPSYS}"
.include <bsd.prog.mk>
.else
${.TARGET}:
	@echo "${OPSYS} platform not supported"
.endif
