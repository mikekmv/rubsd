# $RuOBSD$

PROG=	dynamic
CLEANFILES+=m_a.so m_b.so
MAN=

all:	${PROG} m_a.so m_b.so

m_a.so:	module.c
	$(CC) $(CFLAGS) -DMODULE -DMODULE_NAME=\"A\" -shared -o m_a.so \
	    ${.CURDIR}/module.c

m_b.so:	module.c
	$(CC) $(CFLAGS) -DMODULE -DMODULE_NAME=\"B\" -shared -o m_b.so \
	    ${.CURDIR}/module.c

.include <bsd.prog.mk>
