# $RuOBSD: Makefile.inc,v 1.2 2004/10/31 10:06:45 form Exp $

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

.if defined(SETUID) && ${SETUID:U} == YES
BINGRP=	network
BINMODE=4550
.endif

PREFIX?=/usr/local
BINDIR=${PREFIX}/sbin
