# $RuOBSD$

BINDIR= /usr/local/sbin
MANDIR= /usr/local/man
BINOWN= root
BINGRP= root
BINMODE=755
MANOWN= $(BINOWN)
MANGRP= $(BINGRP)
MANMODE=644

ifneq "$(MAN)" ""
MANINS=	$(MANDIR)/man$(suffix $(MAN))
endif

ifeq "$(SRCS)" ""
SRCS=	$(PROG).c
endif

OBJS=	$(SRCS:.c=.o)

all:	$(PROG)

clean:
	rm -f $(PROG) $(OBJS) $(PROG).core core

install:
	install -o $(BINOWN) -g $(BINGRP) -m $(BINMODE) -s $(PROG) $(BINDIR)
	install -o $(MANOWN) -g $(MANGRP) -m $(MANMODE) $(MAN) $(MANINS)

$(PROG):	$(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LIBS)
