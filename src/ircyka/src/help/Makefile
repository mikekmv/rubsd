# $RuOBSD$

HELPS=	abo.hlp acl.hlp add.hlp bro.hlp bye.hlp del.hlp die.hlp exe.hlp \
	hel.hlp help.hlp inv.hlp joi.hlp ircyka.hlp loa.hlp log.hlp lst.hlp \
	mail.hlp mls.hlp mod.hlp nam.hlp ope.hlp par.hlp psw.hlp set.hlp \
	unl.hlp ver.hlp who.hlp

NOOBJ=	Yes

PREFIX?=/usr/local
DATADIR?=${PREFIX}/lib/ircyka
HELPDIR?=${DATADIR}/help

install:
.if !exists(${DATADIR})
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d \
	    ${DESTDIR}${DATADIR}
.endif
.if !exists(${HELPDIR})
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${DIRMODE} -d \
	    ${DESTDIR}${HELPDIR}
.endif
.for file in ${HELPS}
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m ${NONBINMODE} ${file} \
	    ${DESTDIR}${HELPDIR}
.endfor

.include <bsd.prog.mk>
