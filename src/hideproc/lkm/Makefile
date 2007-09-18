# $RuOBSD$

SUBDIR=	hideproc

.if defined(NO_PROPOLICE) && ${NO_PROPOLICE:U} == YES
MAKE_FLAGS+="NO_PROPOLICE=Yes"
.endif

.include <bsd.subdir.mk>
