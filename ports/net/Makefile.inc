# $RuOBSD: Makefile.inc,v 1.1 2002/08/01 02:28:15 form Exp $

.if !defined(DIST_SUBDIR) || ${DIST_SUBDIR} == ""
DIST_SUBDIR=	LOCAL
.else
DIST_SUBDIR:=	LOCAL/${DIST_SUBDIR}
.endif
