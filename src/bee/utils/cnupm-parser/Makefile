# $Bee$
NOMAN=
PROG=	beeparse
SRCS=	parser.c 
BINDIR=/usr/local/bin
CFLAGS+=-I${.CURDIR} -I../../include -g
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeedb -lbee
LDFLAGS+=-L/usr/local/lib
