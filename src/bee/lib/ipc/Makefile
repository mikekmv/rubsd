# $RuOBSD$
PROG=  bee
SRCS=  ipc.c
LIB=   beeipc
LIBDIR= /usr/local/lib
CFLAGS+=-I${.CURDIR} -I../../include -I/usr/local/include -pthread
COPTS+=	-Wall -Werror 
NOPROFILE=
LDFLAGS+=-L/usr/local/lib
LDADD+= -lbee
.include <bsd.lib.mk>
.include <bsd.own.mk>
