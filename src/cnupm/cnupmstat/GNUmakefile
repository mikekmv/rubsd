# $RuOBSD: GNUmakefile,v 1.1 2004/04/02 14:53:01 form Exp $

PROG=	cnupmstat
MAN=	cnupmstat.8

COPTS=	-Wno-format
LDADD=	-L../libcnupm -lcnupm

include ../GNUosdefs.inc

ifeq "$(OS)" "QNX"
LDADD+=	-lsocket
endif

include ../GNUmakefile.inc
