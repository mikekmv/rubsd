# $RuOBSD$

.ifndef CFLAGS_OK
CFLAGS+=-Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-pedantic -fno-builtin-sin
CFLAGS_OK=
.endif

CPPFLAGS+=-I${.CURDIR}/../../include

MAN?=

PREFIX?=/usr/local
BINDIR?=${PREFIX}/bin
.if ${PREFIX} == "/usr"
MANDIR?=${PREFIX}/share/man/cat
.else
MANDIR?=${PREFIX}/man/cat
.endif
