# $RuOBSD: Makefile.inc,v 1.1.1.1 2004/10/27 06:32:39 form Exp $

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes

CFLAGS+=-I${.CURDIR}/../../include

.if defined(USE_BPF) && ${USE_BPF:U} == YES
CFLAGS+=-DNBPFILTER=1
.endif
