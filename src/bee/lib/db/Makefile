# $RuOBSD: Makefile,v 1.3 2002/01/10 10:56:15 shadow Exp $
PROG=  billd
SRCS=  db.c acc.c log.c
LIB=   beedb
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+= -lz
.include <bsd.lib.mk>
.include <bsd.own.mk>
