# $RuOBSD: Makefile,v 1.5 2004/04/29 07:33:24 form Exp $

PROG=	cnupmstat
MAN=	cnupmstat.8
MAN8=	${MAN}

COPTS=	-Wno-format

.if exists(${.CURDIR}/../libcnupm/obj)
LDADD=	-L${.CURDIR}/../libcnupm/obj -lcnupm
DPADD=	${.CURDIR}/../libcnupm/obj/libcnupm.a
.else
LDADD=	-L${.CURDIR}/../libcnupm -lcnupm
DPADD=	${.CURDIR}/../libcnupm/libcnupm.a
.endif

.include "${.CURDIR}/../BSDosdefs.inc"
.include <bsd.prog.mk>
