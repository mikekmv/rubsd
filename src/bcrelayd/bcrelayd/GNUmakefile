# $RuOBSD$

PROG=	bcrelayd
SRCS=	bcrelayd.c

LDADD=	-lpcap

OS=	$(shell uname)

ifeq "$(OS)" "FreeBSD"
SRCS+=	strtonum.c
endif

ifeq "$(OS)" "Linux"
SRCS+=	strlcat.c strlcpy.c strtonum.c
endif

include ../GNUmakefile.inc
