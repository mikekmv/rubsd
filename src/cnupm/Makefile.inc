# $RuOBSD: Makefile.inc,v 1.2 2004/03/20 06:52:45 form Exp $

COPTS+=	-Wall -Werror -Wmissing-prototypes -Wstrict-prototypes -Winline
COPTS+=	-Wshadow -Wpointer-arith -Wmissing-declarations -Wno-long-long
COPTS+=	-Wno-format
COPTS+=	-I${.CURDIR}/../include
