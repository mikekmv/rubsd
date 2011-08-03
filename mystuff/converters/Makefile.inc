# $RuOBSD: Makefile.inc,v 1.1 2008/07/18 03:48:55 form Exp $

.if !defined(DIST_SUBDIR) || ${DIST_SUBDIR} == ""
DIST_SUBDIR=	LOCAL
.else
DIST_SUBDIR:=	LOCAL/${DIST_SUBDIR}
.endif

MAINTAINER?=	support@openbsd.ru
