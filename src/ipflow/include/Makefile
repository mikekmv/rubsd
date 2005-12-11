# $RuOBSD: Makefile,v 1.1 2005/11/24 17:01:17 form Exp $

INCS=	ipflow.h

NOOBJ=

PREFIX?=/usr/local

install:
.if !exists(${DESTDIR}${PREFIX}/include/net)
	${INSTALL} -m ${DIRMODE} -o ${BINOWN} -g ${BINGRP} -d \
	    ${DESTDIR}${PREFIX}/include/net
.endif
.for i in ${INCS}
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} -o ${BINOWN} -g ${BINGRP} \
	    net/$i ${DESTDIR}${PREFIX}/include/net
.endfor

.include <bsd.prog.mk>
