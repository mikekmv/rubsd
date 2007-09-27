# $RuOBSD: Makefile,v 1.6 2007/09/25 14:49:01 shadow Exp $
PROG=  bee
SRCS=  beelib.c da.c g3c.c lists.c log.c misc.c recode.c timer.c netio.c
LIB=   bee
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include
COPTS+=	-Wall
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
#LDADD+=
.include <bsd.lib.mk>
.include <bsd.own.mk>
