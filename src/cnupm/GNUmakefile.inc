# $RuOBSD: GNUmakefile.inc,v 1.3 2004/04/19 12:53:40 form Exp $

CFLAGS+=-Wall -Werror -Wmissing-prototypes -Wstrict-prototypes -Winline
CFLAGS+=-Wshadow -Wpointer-arith -Wmissing-declarations -Wno-long-long

BINDIR?=	/usr/local/sbin
MANDIR?=	/usr/local/man
BINOWN?=	root
BINGRP?=	root
BINMODE?=	555
MANOWN?=	$(BINOWN)
MANGRP?=	$(BINGRP)
MANMODE?=	444
DIRMODE?=	755

ifeq "$(INSTALL)" ""
INSTALL=	../install-sh
endif

CFLAGS+=	-O2 -I../include
AR=		ar
RANLIB=		ranlib

ifeq "$(SRCS)" ""
ifneq "$(LIB)" ""
SRCS=		lib$(LIB).c
else
SRCS=		$(PROG).c
endif
endif

CFLAGS+=	$(COPTS)
OBJS=		$(SRCS:.c=.o)
CLEANFILES+=	$(OBJS)

ifneq "$(LIB)" ""
CLEANFILES+=	lib$(LIB).a

all:		lib$(LIB).a

install:
	# Nothing here so far...

lib$(LIB).a:	$(OBJS)
	$(AR) cr lib$(LIB).a $(OBJS)
	$(RANLIB) lib$(LIB).a
else
CLEANFILES+=	$(PROG) core *.core

all:		$(PROG)

install:
	@if [ ! -d $(BINDIR) ]; then \
	    $(INSTALL) -c -o $(BINOWN) -g $(BINGRP) -m $(DIRMODE) -d \
		$(BINDIR); \
	fi
	$(INSTALL) -c -o $(BINOWN) -g $(BINGRP) -m $(BINMODE) -s $(PROG) \
	    $(BINDIR)
	@if [ ! -d $(MANDIR)/man8 ]; then \
	    $(INSTALL) -c -o $(MANOWN) -g $(MANGRP) -m $(DIRMODE) -d \
		$(MANDIR)/man8; \
	fi
	$(INSTALL) -c -o $(MANOWN) -g $(MANGRP) -m $(MANMODE) $(MAN) \
	    $(MANDIR)/man8

$(PROG):	$(OBJS)
	$(CC) $(LFDLAGS) -o $(PROG) $(OBJS) $(LDADD)
endif

depend:
	# Nothing here so far...

clean cleandir:
	rm -f $(CLEANFILES)
