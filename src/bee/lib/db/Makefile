# $RuOBSD: Makefile,v 1.5 2004/05/02 21:55:53 shadow Exp $
PROG=  billd
SRCS=  db.c acc.c log.c links.c res.c
LIB=   beedb
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+= -lz
.include <bsd.lib.mk>
.include <bsd.own.mk>
