# $RuOBSD$

LIB=	cnupm
SRCS=	cnupm_daemon.c cnupm_family.c cnupm_pidfile.c cnupm_progname.c
SRCS+=	cnupm_protocol.c cnupm_ulval.c cnupm_version.c

include ../GNUosdefs.inc

ifeq "$(OS)" "Linux"
SRCS+=	setproctitle.c strlcat.c strlcpy.c
endif

ifeq "$(OS)" "QNX"
SRCS+=	setproctitle.c strlcat.c strlcpy.c
endif

include ../GNUmakefile.inc
