# $RuOBSD: Makefile,v 1.5 2004/05/02 21:55:53 shadow Exp $

SRCS=  db.c
LIB=   bee2db
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+= 
.include <bsd.lib.mk>
.include <bsd.own.mk>
