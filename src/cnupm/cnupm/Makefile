# $RuOBSD$

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c
MAN=	cnupm.8
MAN8=	${MAN}

LDADD=	-L${.CURDIR}/../libcnupm/obj -L${.CURDIR}/../libcnupm -lcnupm -lpcap

.include "${.CURDIR}/../BSDosdefs.inc"

.if ${OS} == "FreeBSD"
LDADD+=	-lutil
.endif

.if ${OS} == "NetBSD"
LDADD+=	-lutil
.endif

.include <bsd.prog.mk>
