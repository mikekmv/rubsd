# $RuOBSD$

PROG=	bcrelayd
SRCS=	bcrelayd.c

DPADD=	${LIBPCAP}
LDADD=	-lpcap

OS!=	uname

.if ${OS} == "FreeBSD"
SRCS+=	strtonum.c
.endif

.include <bsd.prog.mk>
