# $RuOBSD: Makefile,v 1.8 2003/08/03 16:44:57 shadow Exp $
NOMAN=
PROG=	payman
SRCS=	cmdview.cc dialog.cc dispatch.cc display.cc      \
        init.cc keybar.cc menu.cc menubar.cc msgbox.cc   \
        panel.cc payman.cc unicon.cc waiter.cc winout.cc \
        da.cc list.cc title.cc userview.cc help.cc       \
        login.cc log.cc inetpay.cc beetalk.cc intrapay.cc

BINDIR=/usr/local/bin/
CFLAGS+= -I../../include
COPTS+=-g -Wall #-Werror

.include <bsd.own.mk>
.include <bsd.prog.mk>

LDADD+= -lcurses -lbee -lbeedb -lbeeipc -lz
LDFLAGS+=-L/usr/local/lib


