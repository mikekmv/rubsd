# $RuOBSD$

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c pidfile.c
MAN=	cnupm.8
MAN8=	${MAN}

CFLAGS+=-Wall -Werror

.include "${.CURDIR}/../Makefile.conf"

DPADD=	${LIBPCAP}
LDADD=	-lpcap

OPSYS!=	uname

.if ${OPSYS} == FreeBSD
CFLAGS+=-Wno-format
COPTS+=	-DLOGIN_CAP
DPADD+=	${LIBUTIL}
LDADD+=	-lutil
.endif

.if ${OPSYS} == OpenBSD
COPTS+=	-DLOGIN_CAP
.else
COPTS+=	-I${.CURDIR}/local
.endif

.include <bsd.prog.mk>
