# $RuOBSD: GNUmakefile,v 1.2 2004/04/19 12:53:42 form Exp $

PROG=	cnupmstat
MAN=	cnupmstat.8

COPTS=	-Wno-format
LDADD=	-L../libcnupm -lcnupm

include ../GNUosdefs.inc

ifeq "$(OS)" "QNX"
LDADD+=	-lsocket
endif

ifeq "$(OS)" "SunOS"
LDADD+=	-lsocket
endif

include ../GNUmakefile.inc
