# $RuOBSD: Makefile,v 1.7 2004/04/24 03:58:38 form Exp $

PROG=	cnupm
SRCS=	cnupm.c collect.c datalinks.c
MAN=	cnupm.8
MAN8=	${MAN}

.if exists(${.CURDIR}/../libcnupm/obj)
LDADD=	-L${.CURDIR}/../libcnupm/obj -lcnupm -lpcap
DPADD=	${.CURDIR}/../libcnupm/obj/libcnupm.a ${LIBPCAP}
.else
LDADD=	-L${.CURDIR}/../libcnupm -lcnupm -lpcap
DPADD=	${.CURDIR}/../libcnupm/libcnupm.a ${LIBPCAP
.endif

.include "${.CURDIR}/../BSDosdefs.inc"

.if ${OS} == "FreeBSD"
LDADD+=	-lutil
DPADD+=	${LIBUTIL}
.endif

.if ${OS} == "NetBSD"
LDADD+=	-lutil
DPADD+=	${LIBUTIL}
.endif

.include <bsd.prog.mk>
