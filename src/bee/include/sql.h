/* $oganer: sql.h,v 1.3 2005/01/31 10:13:55 shadow Exp $	 */

/*
 * Copyright (c) 2003, 2004 Maxim Tsyplakov <tm@oganer.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _SQL_H
#define _SQL_H
#if defined(__MSSQL) || defined(__MYSQL) || defined(__PGSQL)

#include <sys/types.h>

#ifdef __MSSQL
#include <sqlfront.h>
#include <sqldb.h>
#endif				/* MSSQL */

#ifdef __MYSQL
#include <mysql.h>
#endif				/* MYSQL */

#ifdef __PGSQL
#include <postgresql/libpq-fe.h>
#endif				/* PGSQL */

#include "lists.h"

#define SQL_ALLROWS	(-1)
#define SQL_FETCHNEXT   (-2)

#define SQL_FETCH	(0)

#define SQL_SUCCESS	(0)
#define SQL_ERROR	(-1)

#define SQLD_QBUFFSZ	(65536)   /* 256-bytes for query */ 
#define SQLD_FETCHBLKSZ	(512)	/* 512 records by default */
#define SQLD_HOSTNAMESZ	(128)	/* 128 bytes for hostname */

#define TUPLE(T)	((struct sql_tuple *)T)

#define SQL_FOREACH(T, X)	\
	for (X = 0; X < TUPLE(T)->data.count / TUPLE(T)->head.count; ++X)

#define ROW_OFFSET(T, ROW)	(ROW * TUPLE(T)->head.count)

#define TUPLE_FIELD_RAW(T, TYPE, ROW_OFFS, COL)	\
	((TYPE*) pl_get(&TUPLE(T)->data, ROW_OFFS + COL))

#define TUPLE_FIELD(T, ROW_OFFS, COL) \
	((char *)pl_get(&TUPLE(T)->data, ROW_OFFS + COL))
	
struct sql_tuple;

#ifdef __MSSQL
struct mssql_data {
	LOGINREC       *loginrec;
	DBPROCESS      *dbprocess;
};
#endif

#ifdef __MYSQL
struct mysql_data {
	MYSQL          *handle;
	MYSQL_RES      *result;
	int		nextrow;	/* next row to fetch */
	char            qbuff[SQLD_QBUFFSZ];
};
#endif

#ifdef __PGSQL
struct pgsql_data {
	PGconn		*handle;
	PGresult	*result;
	int		nextrow;	/* next row to fetch */
	char		qbuff[SQLD_QBUFFSZ];
}; 
#endif

/* common sql data */
struct sqldata {
	int             type;		/* SQL server type */
	int             modeflags;	/* mode flags */
#define SQLMF_RAWDATA	(0x01)		/* fetching raw result (M$ only) */
#define SQLMF_SSL	(0x02)		/* require SSL (not M$) */

	int             stateflags;	/* state flags */
#define SQLF_INIT	(0x01)		/* initialized */	
#define SQLF_OPENED	(0x02)		/* connection opened */

	char            dbev[256];	/* db error vector */
	char            osev[256];	/* os error vector */
	void	       *data;		/* app level data */
	
	int		cols;		/* result cols       */
	int		rows;		/* result rows       */
	int		count;          /* requested rows count    */
	int		blksz;          /* stored fetch block size */

	/* sql type specific data */
	union {
#ifdef __MSSQL
		struct mssql_data mssql;
#endif

#ifdef __MYSQL
		struct mysql_data mysql;
#endif

#ifdef __PGSQL

		struct pgsql_data pgsql;
#endif

	}               spec;
};

struct sql_tuple {
	struct pl       head;	/* sql_tuple_head pointer list */
	struct pl       data;	/* data: cell0, cell1, ..., cellN */
};

struct sql_tuple_head {
        char           *name;   /* column name */
        int             vtype;  /* for binding */
        int             dsize;  /* max data size */
};

enum sqltypes {
#ifdef  __MSSQL
        SQLTYPE_MSSQL,
#endif
#ifdef __MYSQL
        SQLTYPE_MYSQL,
#endif
#ifdef __PGSQL
        SQLTYPE_PGSQL
#endif
};

__BEGIN_DECLS

/* layer functions  */
struct sqldata  *sql_init(struct sqldata *, int);
int             sql_open(struct sqldata *, char *, char *, char *, char *);
int             sql_close(struct sqldata *);
int             sql_prepare(struct sqldata *, const char *,...);
int             sql_execute(struct sqldata *);
int             sql_getrowscount(struct sqldata *);
int             sql_run(struct sqldata *);
int		sql_fetchhead(struct sqldata *, struct sql_tuple **);
int		sql_fetch(struct sqldata *, struct sql_tuple **, int, int);
int		sql_fetchnext(struct sqldata *, struct sql_tuple **);
int		sql_inittuple(struct sql_tuple *);
int		sql_freetuple(struct sql_tuple *);
int             sql_finish(struct sqldata *);
int		sql_type(const char *);
int             sql_strescape(struct sqldata *, char *, const char *, int);

__END_DECLS

#endif				/* __MSSQL or __MYSQL or __PGSQL */
#endif				/* _SQL_H */
