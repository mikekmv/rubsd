# $RuOBSD: GNUmakefile,v 1.1 2004/04/02 14:53:00 form Exp $

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c
MAN=	cnupm.8

LDADD=	-L../libcnupm -lcnupm -lpcap

include ../GNUosdefs.inc

ifeq "$(OS)" "QNX"
LDADD+=	-lsocket
endif

include ../GNUmakefile.inc

