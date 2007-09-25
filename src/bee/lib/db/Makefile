# $RuOBSD: Makefile,v 1.7 2007/09/20 09:59:58 shadow Exp $
PROG=  billd
SRCS=  db.c acc.c log.c links.c res.c tariffs.c conf.c
LIB=   beedb
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
.include <bsd.lib.mk>
.include <bsd.own.mk>
