# $RuOBSD: Makefile,v 1.1 2003/07/04 12:50:54 shadow Exp $
NOMAN=
PROG=	payman
SRCS=	cmdview.cc dialog.cc dispatch.cc display.cc      \
        init.cc keybar.cc menu.cc menubar.cc msgbox.cc   \
        panel.cc payman.cc unicon.cc waiter.cc winout.cc \
        da.cc list.cc

BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include
COPTS+=-g -Wall -Werror

.include <bsd.own.mk>
.include <bsd.prog.mk>

LDADD+= -lcurses
LDFLAGS+=-L/usr/local/lib

