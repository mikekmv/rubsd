# $RuOBSD: Makefile.inc,v 1.1 2002/12/18 03:35:08 form Exp $

.ifndef CFLAGS_OK
CFLAGS+=-W -Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-pedantic
CFLAGS_OK=
.endif

BINDIR=	/usr/libexec
