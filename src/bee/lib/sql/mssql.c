/* $oganer: mssql.c,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

/*
 * Copyright (c) 2004 Maxim Tsyplakov <tm@oganer.net>
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

#if defined(__MSSQL)
#include <sys/param.h>
#include <sys/utsname.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sqlfront.h>
#include <sqldb.h>
#include <sybdb.h>

#include "lists.h"
#include "misc.h"
#include "sql.h"
#include "internal.h"

#define FIXCOLLEN	256
#define SETDBERR(D, S)	{ strlcpy((D)->dbev, S, sizeof (D)->dbev); }
#define SETOSERR(D, S)  { strlcpy((D)->osev, S, sizeof (D)->osev); }
#define SETOSERRNO(D, C) do {	\
	char *_tmp;		\
	_tmp = strerror(C);	\
	SETOSERR((D), _tmp);	\
} while (0);

struct mssqltt {
	int             stype;	/* sql type */
	int             vtype;	/* for binding */
	int             size;
};

int             mssql_init(struct sqldata *);
int             mssql_open(struct sqldata *, char *, char *, char *, char *);
int             mssql_prepare(struct sqldata *, const char *);
int             mssql_execute(struct sqldata *);
int             mssql_fetch(struct sqldata *, int, int);
int             mssql_run(struct sqldata *);
int             mssql_finish(struct sqldata *);
int             mssql_close(struct sqldata *);

static int	cmp_mssqltt(const void *, const void *);
static int      mssql_err_handler(DBPROCESS *, DBINT, int, int, char *, char *);
static int 	mssql_msg_handler(DBPROCESS *, int, int, int, char *, char *,
		  char *, int);
		  
static int	fill_head(struct sql_tuple *, char *, int, int, int);
static int	addrow(struct sqldata *, struct sql_tuple *);

static struct mssqltt  *find_trans(struct mssqltt *, int, int);

static struct mssqltt mssql_transtable[] = {
	{SYBBIT, BITBIND, sizeof(DBCHAR)},
	{SYBCHAR, STRINGBIND, sizeof(DBCHAR)},
	{SYBVARCHAR, STRINGBIND, sizeof(DBCHAR)},
	{SYBTEXT, STRINGBIND, sizeof(DBCHAR)},
	{SYBBINARY, BINARYBIND, sizeof(DBBINARY)},
	{SYBVARBINARY, BINARYBIND, sizeof(DBBINARY)},
	{SYBIMAGE, BINARYBIND, sizeof(DBBINARY)},
	{SYBINT1, TINYBIND, sizeof(DBTINYINT)},
	{SYBINT2, SMALLBIND, sizeof(DBSMALLINT)},
	{SYBINT4, INTBIND, sizeof(DBINT)},
	{SYBINTN, INTBIND, sizeof(DBINT)},
	{SYBFLT8, FLT8BIND, sizeof(DBFLT8)},
	{SYBFLTN, FLT8BIND, sizeof(DBFLT8)},
	{SYBMONEY4, SMALLMONEYBIND, sizeof(DBMONEY4)},
	{SYBMONEY, MONEYBIND, sizeof(DBMONEY)},
	{SYBMONEYN, MONEYBIND, sizeof(DBMONEY)},
	{SYBDECIMAL, DECIMALBIND, sizeof(DBNUMERIC)},
	{SYBNUMERIC, NUMERICBIND, sizeof(DBNUMERIC)},
	{SYBDATETIME4, SMALLDATETIMEBIND, sizeof(DBDATETIME4)},
	{SYBDATETIME, DATETIMEBIND, sizeof(DBDATETIME)},
#define TTCNT	(sizeof mssql_transtable /sizeof mssql_transtable[0])
};

struct optb     optb_mssql = {
	mssql_init,
	mssql_open,
	mssql_close,
	mssql_prepare,
	mssql_execute,
	mssql_run,
	mssql_fetch,
	mssql_finish
};

static int
cmp_mssqltt(const void * v0, const void * v1)
{
	return (((struct mssqltt *)v0)->stype - ((struct mssqltt *)v0)->stype);
}

int
mssql_init(struct sqldata * data)
{
	RETCODE         rc;

	memset(data->dbev, '\0', sizeof data->dbev);
	memset(data->osev, '\0', sizeof data->osev);

	/* sort t table */
	qsort(mssql_transtable, TTCNT, sizeof (struct mssqltt), cmp_mssqltt);

	/* initialize DB-library */
	rc = dbinit();
	if (rc != SUCCEED) {
		SETDBERR(data, "dbinit() error in mssql_init()");
		return (SQL_ERROR);
	}
	dberrhandle(mssql_err_handler);
	dbmsghandle(mssql_msg_handler);
	return (SQL_SUCCESS);
}

int
mssql_open(struct sqldata * data, char *server, char *dbname, char *login,
	   char *password)
{
	struct utsname  ut;

	if (data->spec.mssql.loginrec == NULL)
		data->spec.mssql.loginrec = dblogin();

	if (data->spec.mssql.loginrec == NULL ||
	    server == NULL || login == NULL ||
	    password == NULL || dbname == NULL) {
		SETOSERR(data, "invalid parameter");
		return (SQL_ERROR);
	}
	if (uname(&ut))
		memset(&ut.nodename, '\0', sizeof ut.nodename);

	if (DBSETLUSER(data->spec.mssql.loginrec, login) == FAIL ||
	    DBSETLPWD(data->spec.mssql.loginrec, password) == FAIL ||
	    DBSETLHOST(data->spec.mssql.loginrec, ut.nodename) == FAIL ||
	    DBSETLAPP(data->spec.mssql.loginrec, __progname) == FAIL)
		return (SQL_ERROR);

	data->spec.mssql.dbprocess = dbopen(data->spec.mssql.loginrec, server);

	if (data->spec.mssql.dbprocess == NULL)
		return (SQL_ERROR);

	if (dbuse(data->spec.mssql.dbprocess, dbname) == FAIL)
		return (SQL_ERROR);

	dbsetuserdata(data->spec.mssql.dbprocess, (BYTE *) data);

	return (SQL_SUCCESS);
}

int
mssql_prepare(struct sqldata * data, const char *buf)
{
	struct mssql_data *spec;

	spec = &data->spec.mssql;
	return (dbcmd(spec->dbprocess, buf) == FAIL ? SQL_ERROR : SQL_SUCCESS);
}

int
mssql_fetch(struct sqldata * data, int count, int blksz)
{
	struct mssql_data *spec;
	struct sql_tuple *tuple;
	struct sql_tuple_head *head;
	int             cols, type, len;
	int             i, rc;
	RETCODE         retc;

	spec = &data->spec.mssql;

	/* allocate tuple */
	tuple = calloc(1, sizeof(struct sql_tuple));

	if (tuple == NULL) {
		SETOSERRNO(data, errno)
		return (NULL);
	}
	pl_init(&tuple->data, blksz, pl_def_dealloc);
	cols = dbnumcols(spec->dbprocess);

	if (cols <= 0)
		return (SQL_SUCCESS);	/* stub */
	
	pl_init(&tuple->head, cols, head_free);

	/* initialize tuple head */
	for (i = 0; i < cols; ++i) {
		head = calloc(1, sizeof(struct sql_tuple_head));
		if (head == NULL) {
			SETOSERRNO(data, errno);
			data->proc(data, NULL, SQL_ERROR, SQL_SUCCESS);
			goto error;
		}
		pl_add(&tuple->head, head);
		if (tuple->head.rc != LST_ESUCCESS) {
			SETOSERRNO(data, errno);
			data->proc(data, NULL, SQL_ERROR, SQL_SUCCESS);			
			free(head);
			goto error;
		}
	}

	/* fill tuple's head */
	for (i = 0; i < cols; ++i) {
		type = dbcoltype(spec->dbprocess, i + 1);
		len = dbcollen(spec->dbprocess, i + 1);
		if (fill_head(tuple, dbcolname(spec->dbprocess, i + 1), type,
		    len, i) == -1) {
			SETOSERRNO(data, errno);
			data->proc(data, NULL, SQL_ERROR, SQL_SUCCESS);			
			goto error;			
		}
	}

	i = 0;
	for (;;) {
		rc = addrow(data, tuple);
		if (rc == SQL_ERROR)
			goto error; /* error set in addrow */
		retc = dbnextrow(spec->dbprocess);
		if (retc == FAIL)
			goto error;
		if (retc == NO_MORE_ROWS) {
			if (tuple->data.count > 0) {
				for (; cols; --cols) {
					pl_delete(&tuple->data,
						tuple->data.count - 1);
				}
				data->proc(data, tuple, SQL_FETCH, SQL_SUCCESS);
			}
			break;
		}
		++i;
		if (i == blksz) {
			data->proc(data, tuple, SQL_FETCH, SQL_SUCCESS);
			pl_clear(&tuple->data);
			i = 0;
		}
		if (count != SQL_ALLROWS && --count == 0)
			break;
	}
	return (SQL_SUCCESS);

error:
	tuple_free(tuple);
	dbcancel(spec->dbprocess);
	return (SQL_ERROR);
}

int
mssql_execute(struct sqldata * data)
{

	struct mssql_data *spec;
	RETCODE         rc;

	spec = &data->spec.mssql;

	rc = dbsqlexec(spec->dbprocess);
	if (rc == FAIL)
		return (SQL_ERROR);

	rc = dbresults(spec->dbprocess);
	if (rc == FAIL)
		return (SQL_ERROR);

	return (SQL_SUCCESS);
}

int
mssql_run(struct sqldata * data)
{
	struct mssql_data *spec;
	RETCODE         retc;

	spec = &data->spec.mssql;

	if (dbsqlexec(spec->dbprocess) == FAIL)
		return (SQL_ERROR);

	do {
		retc = dbresults(spec->dbprocess);
		if (retc == NO_MORE_RESULTS)
			break;
		if (retc == FAIL) {
			dbcancel(spec->dbprocess);
			return (SQL_ERROR);
		}
		do {
			retc = dbnextrow(spec->dbprocess);
			if (retc == FAIL) {
				dbcancel(spec->dbprocess);
				return (SQL_ERROR);
			}
		} while (retc != NO_MORE_ROWS);
	} while (1);

	return (SQL_SUCCESS);
}

int
mssql_finish(struct sqldata * data)
{
	struct mssql_data *spec;

	spec = &data->spec.mssql;
	dbfreebuf(spec->dbprocess);
	return (SQL_SUCCESS);
}

int
mssql_close(struct sqldata * data)
{
	struct mssql_data *spec;
	
	spec = &data->spec.mssql;
	dbclose(spec->dbprocess);
	return (SQL_SUCCESS);
}

static int
mssql_err_handler(DBPROCESS * dbproc, int severity, int dberr, int oserr,
		  char *dberrstr, char *oserrstr)
{
	struct sqldata *data;
	struct mssql_data *spec;

	data = (struct sqldata *) dbgetuserdata(dbproc);
	if (data != NULL)
		goto exit;

	spec = &data->spec.mssql;

	if (dberrstr != NULL)
		SETDBERR(data, dberrstr);
	if (oserrstr != NULL)
		SETOSERR(data, oserrstr);

	if (dberr != DBNOERR || oserr != DBNOERR)
		data->proc(data, NULL, SQL_ERROR, SQL_SUCCESS);

exit:
	if ((dbproc == NULL) || (DBDEAD(dbproc)))
		return (INT_EXIT);
	else
		return (INT_CANCEL);
}

static int
mssql_msg_handler(DBPROCESS * dbproc, DBINT msgno, int msgstate,
	   int severity, char *msgtext, char *srvname, char *proc, int line)
{
	return (0);
}

static int
fill_head(struct sql_tuple * tuple, char *name, int type, int len, int col)
{

	struct sql_tuple_head *head;
	struct mssqltt *item;

	head = pl_get(&tuple->head, col);
	head->name = strnalloc(name, 0);
	if (head->name == NULL)
		return (SQL_ERROR);

	head->dsize = len;

	item = find_trans(mssql_transtable, type, TTCNT);
	head->vtype = item != NULL ? item->vtype : SQL_ERROR;

	return (SQL_SUCCESS);
}

static int
addrow(struct sqldata * data, struct sql_tuple * tuple)
{
	struct mssql_data *spec;
	struct sql_tuple_head *head;	
	void           *slice;
	RETCODE         retc;
	int             i;
	int             al;	/* actual length */
	static char colbuf[FIXCOLLEN];

	spec = &data->spec.mssql;		
	if (data->flags & SQLF_RAWDATA)	{		
		for (i = 0; i < tuple->head.count; ++i) {				
			head = pl_get(&tuple->head, i);
		
			slice = calloc(1, head->dsize);
		
			if (slice == NULL) {
				SETOSERRNO(data, errno);
				return (SQL_ERROR);
			}
			pl_add(&tuple->data, slice);
			if (tuple->data.rc != LST_ESUCCESS) {
				SETOSERRNO(data, errno);
				free(slice);
				return (SQL_ERROR);
			}
				
			switch (head->vtype) {
			case STRINGBIND:
			case BINARYBIND:
				al = head->dsize + 1;
				break;
			default:
				al = head->dsize;
			}

			retc = dbbind(spec->dbprocess, i + 1, head->vtype, al, slice);
			if (retc == FAIL) {
				SETOSERRNO(data, errno);
				free(slice);
				return (SQL_ERROR);
			}
		}
	} else {
		for (i = 0; i < tuple->head.count; ++i) {
			head = pl_get(&tuple->head, i);
			if (head->vtype == STRINGBIND || head->vtype == BINARYBIND) {				
				slice = calloc(1, head->dsize);
				if (slice == NULL) {
					SETOSERRNO(data, errno);
					return (SQL_ERROR);
				}	
				retc = dbbind(spec->dbprocess, i + 1, STRINGBIND, head->dsize + 1, slice);
				if (retc == FAIL) {
					free(slice);
					return (SQL_ERROR);
				}
			} else {
				slice = &colbuf;
				memset(slice, sizeof colbuf, 0);	
				retc = dbbind(spec->dbprocess, i + 1, STRINGBIND, sizeof colbuf, slice);
				if (retc == FAIL) {
					free(slice);
					return (SQL_ERROR);
				}
				slice = strdup(slice);
				if (slice == NULL) {
					SETOSERRNO(data, errno);
					return (SQL_ERROR);
				}
				head->dsize = strlen(slice);	
			}
			pl_add(&tuple->data, slice);
			if (tuple->data.rc != LST_ESUCCESS) {
				SETOSERRNO(data,  tuple->data.rc);
				free(slice);
				return (SQL_ERROR);
			}					
		}
	}	
	return (SQL_SUCCESS);
}

static struct mssqltt *
find_trans(struct mssqltt * table, int type, int size)
{
	struct mssqltt find;
	
	find.stype = type;
	return (bsearch(&find, table, size, sizeof find, cmp_mssqltt));
}

#endif				/* __MSSQL */
