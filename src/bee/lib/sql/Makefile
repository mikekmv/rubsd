# $oganer: Makefile,v 1.1 2005/05/09 14:47:03 shadow Exp $

LIB=	beesql
SRCS=	sql.c

MSSQL?=		No
PGSQL?=		Yes
MYSQL?=		No

.if ${MSSQL} == Yes
CFLAGS+=-D__MSSQL
SRCS +=	mssql.c
.endif

.if ${MYSQL} == Yes
CFLAGS+=-D__MYSQL -I/usr/local/include/mysql
SRCS += mysql.c
.endif

.if ${PGSQL} == Yes
CFLAGS+= -D__PGSQL -I/usr/local/include/postgresql
SRCS +=	pgsql.c
.endif

CFLAGS+=-Wmissing-prototypes -Wall
NOPROFILE=
NOPIC=

.include <bsd.lib.mk>
