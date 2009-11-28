# $RuOBSD: Makefile,v 1.8 2007/09/25 14:49:01 shadow Exp $
PROG=  billd
SRCS=  db.c acc.c log.c links.c res.c tariffs.c conf.c
LIB=   beedb
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall -Werror -pg
#NOPROFILE=
LDFLAGS+=-L/usr/local/lib
.include <bsd.lib.mk>
.include <bsd.own.mk>
