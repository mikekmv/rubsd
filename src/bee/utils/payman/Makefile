# $RuOBSD: Makefile,v 1.2 2001/09/11 03:14:23 shadow Exp $
NOMAN=
PROG=	payman
SRCS=	cmdview.cc dialog.cc dispatch.cc display.cc \
        init.cc keybar.cc menu.cc menubar.cc msgbox.cc \
        panel.cc payman.cc unicon.cc waiter.cc winout.cc

BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include
COPTS+=-g -Wall #-Werror

.include <bsd.own.mk>
.include <bsd.prog.mk>

LDADD+= -lcurses
LDFLAGS+=-L/usr/local/lib

