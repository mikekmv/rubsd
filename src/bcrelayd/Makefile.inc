# $RuOBSD$

CFLAGS+=-W -Wall -Werror -Winline -Wshadow -Wcast-qual -Wpointer-arith
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes
CFLAGS+=-pedantic -Wno-long-long

MAN?=

BINDIR?=/usr/local/sbin
MANDIR?=/usr/local/man/cat
