# $RuOBSD: GNUmakefile,v 1.3 2004/04/22 03:17:56 form Exp $

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c
MAN=	cnupm.8

LDADD=	-L../libcnupm -lcnupm -lpcap

include ../GNUosdefs.inc

ifeq "$(OS)" "FreeBSD"
LDADD+=	-lutil
endif

ifeq "$(OS)" "NetBSD"
LDADD+=	-lutil
endif

ifeq "$(OS)" "QNX"
LDADD+=	-lsocket
endif

ifeq "$(OS)" "SunOS"
LDADD+=	-lsocket
endif

include ../GNUmakefile.inc

