# $RuOBSD$

CFLAGS+=-W -Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes

CFLAGS+=-I${.CURDIR}/../../include

NOLINT=
NOPIC=
NOPROFILE=

install:
