# $RuOBSD: Makefile,v 1.1.1.1 2001/09/28 09:17:12 tm Exp $

PROG=radioctl

MAN9=radio.9
MAN4=radio.4 az.4 sf2r.4 sf4r.4
MAN1=radioctl.1

MAN=${MAN1} ${MAN4} ${MAN9}

# Custom targets, to install into /usr/local and clean
PREFIX=/usr/local

install: all maninstall
	${INSTALL} ${INSTALL_COPY} ${INSTALL_STRIP} -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} ${PROG} ${PREFIX}/bin/${PROG}

distclean: clean cleanman

.include <bsd.man.mk>
.include <bsd.prog.mk>
