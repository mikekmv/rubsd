# $RuOBSD$

PROG=	cnupmstat
MAN=	cnupmstat.8
MAN8=	${MAN}

COPTS=	-Wno-format
LDADD=	-L${.CURDIR}/../libcnupm/obj -L${.CURDIR}/../libcnupm -lcnupm

.include "${.CURDIR}/../BSDosdefs.inc"
.include <bsd.prog.mk>
