# $RuOBSD: Makefile,v 1.8 2007/09/24 12:57:43 shadow Exp $
NOMAN=
PROG=	bee
SRCS=	core.c command.c res.c sqlcfg.c queries.c
BINDIR=/usr/local/bin/
CFLAGS+=-I${.CURDIR} -I../include -g
COPTS+=-Wall -Werror
.include <bsd.own.mk>
.include <bsd.prog.mk>
LDADD+= -lbeeipc -lbeedb -lz -lbee2db -lbeesql -lpq -lbee
LDFLAGS+=-L/usr/local/lib

LDSRCDIR = sql
LIBDATA  = scripts.dat
LDOWN    = root
LDGRP    = wheel
LDMODE   = 700
LDDIR    = /usr/local/libdata/bee

libdata:
	mkdir -p ${LDDIR}
	for name in ${LIBDATA} ; do \
            ${INSTALL} ${INSTALL_COPY} -o ${LDOWN} -g ${LDGRP} \
            -m ${LDMODE} ${LDSRCDIR}/$${name} ${DESTDIR}${LDDIR}; \
            done;
createdb:
	@psql -q -f sql/db_create.sql beedb bee
	@psql -q -f sql/db_procs.sql beedb bee

destroydb:
	@psql -q -f sql/db_destroy.sql beedb bee

update-procs:
	@psql -q -f sql/db_procs.sql beedb bee

