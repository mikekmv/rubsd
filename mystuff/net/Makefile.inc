# $RuOBSD$

.if !defined(DIST_SUBDIR) || ${DIST_SUBDIR} == ""
DIST_SUBDIR=	LOCAL
.else
DIST_SUBDIR:=	LOCAL/${DIST_SUBDIR}
.endif

MAINTAINER?=	support@openbsd.ru
