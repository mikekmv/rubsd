# $RuOBSD: Makefile,v 1.2 2003/07/14 11:17:25 shadow Exp $
NOMAN=
PROG=	payman
SRCS=	cmdview.cc dialog.cc dispatch.cc display.cc      \
        init.cc keybar.cc menu.cc menubar.cc msgbox.cc   \
        panel.cc payman.cc unicon.cc waiter.cc winout.cc \
        da.cc list.cc title.cc

BINDIR=/usr/local/bin/
CFLAGS+= -I../../include
COPTS+=-g -Wall #-Werror

.include <bsd.own.mk>
.include <bsd.prog.mk>

LDADD+= -lcurses -lbee -lbeedb -lbeeipc
LDFLAGS+=-L/usr/local/lib


