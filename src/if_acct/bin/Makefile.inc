# $RuOBSD: Makefile.inc,v 1.1.1.1 2004/10/27 06:32:39 form Exp $

CFLAGS+=-W -Wall -Werror -Winline -Wpointer-arith -Wmissing-declarations
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes

CFLAGS+=-I${.CURDIR}/../../include

.if exists(${.CURDIR}/../../lib/libflow/obj/libflow.a)
DPADD+=	${.CURDIR}/../../lib/libflow/obj/libflow.a
LDADD+=	-L${.CURDIR}/../../lib/libflow/obj -lflow
.else
DPADD+=	${.CURDIR}/../../lib/libflow/libflow.a
LDADD+=	-L${.CURDIR}/../../lib/libflow -lflow
.endif
