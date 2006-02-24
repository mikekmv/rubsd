# $RuOBSD$

.ifndef CFLAGS_OK
CFLAGS+=-Wall -Werror -Wshadow -Winline -Wpointer-arith -Wcast-qual
CFLAGS+=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=-pedantic -fpic
CFLAGS+=${PICFLAG}
CFLAGS_OK=
.endif

.ifndef LDFLAGS_OK
LDFLAGS+=--shared
LDFLAGS_OK=
.endif

CPPFLAGS+=-I${.CURDIR}/../../include

PREFIX?=/usr/local
DATADIR?=${PREFIX}/lib/ircyka
BINDIR?=${DATADIR}/modules
BINMODE?=${LIBMODE}

INSTALL_STRIP=

PROG?=	${MOD}
SRCS?=	${PROG:R}.c
MAN?=

beforeinstall:
.if !exists(${DATADIR})
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d \
	    ${DATADIR}
.endif
.if !exists(${BINDIR})
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d \
	    ${BINDIR}
.endif

${PROG}: ${OBJS} ${DPADD}
	${LD} -x ${LDFLAGS} -o ${PROG} ${OBJS} ${LDADD}
