# $RuOBSD: GNUmakefile,v 1.2 2004/04/19 12:53:41 form Exp $

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c
MAN=	cnupm.8

LDADD=	-L../libcnupm -lcnupm -lpcap

include ../GNUosdefs.inc

ifeq "$(OS)" "QNX"
LDADD+=	-lsocket
endif

ifeq "$(OS)" "SunOS"
LDADD+=	-lsocket
endif

include ../GNUmakefile.inc

