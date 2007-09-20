# $RuOBSD: Makefile,v 1.6 2007/09/15 11:03:31 shadow Exp $
PROG=  billd
SRCS=  db.c acc.c log.c links.c res.c tariffs.c
LIB=   beedb
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+= -lz
.include <bsd.lib.mk>
.include <bsd.own.mk>
