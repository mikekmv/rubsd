# $oganer: Makefile,v 1.1 2002/11/18 02:58:35 shadow Exp $

PROG=	ipchk

SRCS=   ipchk.c misc.c

#MANDIR=		/usr/local/man/man
NOMAN=		NOMAN
BINOWN=		root
BINGRP=		bin
BINMODE=	555
BINDIR=	/usr/local/sbin
#CFLAGS+= -g
COPTS+=	-Wall -Werror 

#INSTALL_STRIP=

.include <bsd.own.mk>

#LDFLAGS+=-L/usr/local/lib

.include <bsd.prog.mk>

