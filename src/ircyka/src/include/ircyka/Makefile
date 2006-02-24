# $RuOBSD$

INCS=	acldb.h ircyka.h nickdb.h

NOOBJ=	Yes

.include <bsd.own.mk>

install:
.if !exists(${INCDIR}/ircyka)
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d \
	    ${DESTDIR}${INCDIR}/ircyka
.endif
.for file in ${INCS}
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${NONBINMODE} ${file} \
	    ${DESTDIR}${INCDIR}/ircyka
.endfor

.include <bsd.prog.mk>
