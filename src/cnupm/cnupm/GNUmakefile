# $RuOBSD$

PROG=	cnupm
MAN=	cnupm.8
SRCS=	cnupm.c collect.c datalinks.c pidfile.c strlcxx.c
LIBS=	-lpcap

CFLAGS+=-I/usr/include/pcap -Ilocal -DNO_SETPROCTITLE -DLOCAL_STRLCXX
CFLAGS+=-Dth_sport=source -Dth_dport=dest -Duh_sport=source -Duh_dport=dest

include ../GNUmakefile.inc
