# $RuOBSD: Makefile,v 1.3 2004/03/19 03:17:46 form Exp $

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c pidfile.c
MAN=	cnupm.8
MAN8=	${MAN}

CFLAGS+=-Wall -Werror -ansi

DPADD=	${LIBPCAP}
LDADD=	-lpcap

OPSYS!=	uname

.if ${OPSYS} == FreeBSD
COPTS+=	-DLOGIN_CAP
DPADD+=	${LIBUTIL}
LDADD+=	-lutil
.endif

.if ${OPSYS} == NetBSD
CFLAGS+=-pedantic
.endif

.if ${OPSYS} == OpenBSD
CFLAGS+=-pedantic
COPTS+=	-DLOGIN_CAP
.else
CFLAGS+=-Wno-format
COPTS+=	-I${.CURDIR}/local
.endif

.include <bsd.prog.mk>
