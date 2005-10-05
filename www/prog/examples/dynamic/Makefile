# $RuOBSD: Makefile,v 1.2 2004/11/21 10:17:20 form Exp $

PROG=	dynamic
CLEANFILES+=m_a.so m_b.so
MAN=

CFLAGS+=-W -Wall -Werror -Winline -Wshadow -Wcast-qual -Wpointer-arith
CFLAGS+=-Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes

all:	${PROG} m_a.so m_b.so

m_a.so:	module.c
	$(CC) $(CFLAGS) -fpic -DPIC -DMODULE -DMODULE_NAME=\"A\" -shared \
	    -o m_a.so ${.CURDIR}/module.c

m_b.so:	module.c
	$(CC) $(CFLAGS) -fpic -DPIC -DMODULE -DMODULE_NAME=\"B\" -shared \
	    -o m_b.so ${.CURDIR}/module.c

.include <bsd.prog.mk>
