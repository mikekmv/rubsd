# $RuOBSD: Makefile,v 1.1 2004/11/01 05:00:44 form Exp $

PROG=	dynamic
CLEANFILES+=m_a.so m_b.so
MAN=

all:	${PROG} m_a.so m_b.so

m_a.so:	module.c
	$(CC) $(CFLAGS) -fpic -DPIC -DMODULE -DMODULE_NAME=\"A\" -shared \
	    -o m_a.so ${.CURDIR}/module.c

m_b.so:	module.c
	$(CC) $(CFLAGS) -fpic -DPIC -DMODULE -DMODULE_NAME=\"B\" -shared \
	    -o m_b.so ${.CURDIR}/module.c

.include <bsd.prog.mk>
