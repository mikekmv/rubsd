# $RuOBSD: Makefile,v 1.10 2004/11/05 12:32:02 form Exp $

PROG=	cnupm
SRCS=	aggr_port.c cnupm.c collect.c datalinks.c
MAN=	cnupm.8
MAN8=	${MAN}

CFLAGS+=-finline-limit-1000

.if exists(${.CURDIR}/../libcnupm/obj)
LDADD=	-L${.CURDIR}/../libcnupm/obj -lcnupm -lpcap
DPADD=	${.CURDIR}/../libcnupm/obj/libcnupm.a ${LIBPCAP}
.else
LDADD=	-L${.CURDIR}/../libcnupm -lcnupm -lpcap
DPADD=	${.CURDIR}/../libcnupm/libcnupm.a ${LIBPCAP}
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
