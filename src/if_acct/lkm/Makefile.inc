# $RuOBSD: Makefile.inc,v 1.2 2004/10/27 10:16:16 form Exp $

CFLAGS+=-Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes

CFLAGS+=-I${.CURDIR}/../../include

.if defined(USE_BPF) && ${USE_BPF:U} == YES
CFLAGS+=-DNBPFILTER=1
.endif

INCDIR=	/usr/local/include/net

afterinstall: ${INCDIR}
	install -o ${BINOWN} -g ${BINGRP} -m ${NONBINMODE} \
	    ${.CURDIR}/../../include/net/if_acct.h ${INCDIR}

${INCDIR}:
	install -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d ${INCDIR}
