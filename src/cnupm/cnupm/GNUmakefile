# $RuOBSD: GNUmakefile,v 1.6 2004/11/12 22:42:24 form Exp $

PROG=	cnupm
SRCS=	aggr_port.c cnupm.c collect.c datalinks.c
MAN=	cnupm.8

CFLAGS+=-finline-limit-1000

LDADD=	-L../libcnupm -lcnupm -lpcap

include ../GNUosdefs.inc

ifeq "$(OS)" "FreeBSD"
LDADD+=	-lutil
endif

ifeq "$(OS)" "NetBSD"
LDADD+=	-lutil
endif

ifeq "$(OS)" "QNX"
LDADD+=	-lsocket -L/opt/lib
endif

ifeq "$(OS)" "SunOS"
LDADD+=	-lsocket
endif

include ../GNUmakefile.inc

