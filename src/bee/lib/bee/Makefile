# $RuOBSD: Makefile,v 1.2 2001/09/12 05:03:20 tm Exp $
PROG=  bee
SRCS=  beelib.c da.c
LIB=   bee
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include -pthread
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+=
.include <bsd.lib.mk>
.include <bsd.own.mk>
