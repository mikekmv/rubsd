# $RuOBSD$

CFLAGS+=-Wall -Werror -Winline -Wshadow -Wcast-qual -Wpointer-arith
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes

CPPFLAGS+=-I${.CURDIR}/../../include

MAN?=

BINDIR?=/usr/local/sbin
MANDIR?=/usr/local/man/cat
