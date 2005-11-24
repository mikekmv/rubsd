# $RuOBSD$

INCS=	ipflow.h

NOOBJ=

PREFIX?=/usr/local

install:
.for i in ${INCS}
	${INSTALL} ${INSTALL_COPY} -m ${NONBINMODE} $i \
	    ${DESTDIR}${PREFIX}/include
.endfor

.include <bsd.prog.mk>
