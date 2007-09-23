# $RuOBSD: Makefile,v 1.3 2004/05/08 15:35:55 shadow Exp $
PROG=  bee
SRCS=  beelib.c da.c timer.c
LIB=   bee
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include -pthread
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+=
.include <bsd.lib.mk>
.include <bsd.own.mk>
