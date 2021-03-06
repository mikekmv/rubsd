# $RuOBSD$

.ifndef CFLAGS_OK
CFLAGS+=-W -Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-pedantic
CFLAGS_OK=
.endif

CPPFLAGS+=-I${.CURDIR}/../../include

MAN?=

PREFIX?=/usr/local
BINDIR?=${PREFIX}/sbin
MANDIR?=${PREFIX}/man/cat
