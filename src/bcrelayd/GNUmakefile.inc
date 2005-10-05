# $RuOBSD$

CFLAGS+=-W -Wall -Werror -Winline -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-pedantic -Wno-long-long

MAN?=

BINDIR?=        /usr/local/sbin
MANDIR?=        /usr/local/man
BINOWN?=        root
BINGRP?=        root
BINMODE?=       555
MANOWN?=        $(BINOWN)
MANGRP?=        $(BINGRP)
MANMODE?=       444
DIRMODE?=       755

ifeq "$(SRCS)" ""
SRCS=		$(PROG).c
endif

CFLAGS+=	$(COPTS)
OBJS=		$(SRCS:.c=.o)
CLEANFILES+=	$(OBJS)

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
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) $(LDADD)

depend:
	# Nothing here so far...

clean cleandir:
	rm -f $(CLEANFILES)
