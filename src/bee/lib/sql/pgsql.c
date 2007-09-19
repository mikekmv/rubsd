/* $oganer: pgsql.c,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

/*
 * Copyright (c) 2003, 2004 Maxim Tsyplakov <tm@oganer.net>
 *			    Pavel Varnavsky <bss@oganer.net>	
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

#if defined(__PGSQL)
#include <sys/param.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"
#include "misc.h"
#include "sql.h"

int             pgsql_init(struct sqldata *);
int             pgsql_open(struct sqldata *, char *, int, char *, char *, char *);
int             pgsql_prepare(struct sqldata *, const char *);
int             pgsql_fetchhead(struct sqldata *, struct sql_tuple *);
int             pgsql_fetch(struct sqldata *, struct sql_tuple *);
int             pgsql_execute(struct sqldata *);
int             pgsql_run(struct sqldata *);
int             pgsql_finish(struct sqldata *);
int             pgsql_close(struct sqldata *);
int             pgsql_strescape(char *, const char *, int);

struct optb     optb_pgsql = {
	pgsql_init,
	pgsql_open,
	pgsql_close,
	pgsql_prepare,
	pgsql_execute,
	pgsql_run,
	pgsql_fetchhead,
	pgsql_fetch,
	pgsql_finish,
	pgsql_strescape
};

int
pgsql_init(struct sqldata * data)
{
	return (SQL_SUCCESS);
}

int
pgsql_open(struct sqldata * data, char *host, int port, char *dbName, 
	char *login, char *pwd)
{
	char           *coninfo;
	struct pgsql_data *spec;

	spec = &data->spec.pgsql;

	if (dbName == NULL || login == NULL)
		return (SQL_ERROR);

        if (port <= 0 || port > 65534) port = 5432;

        if (*host == '/') {
	    asprintf(&coninfo, "host=%s dbname=%s user=%s password=%s connect_timeout=0",
		 host, dbName, login, pwd);
	}
	else {
	    asprintf(&coninfo, "host=%s port=%d dbname=%s user=%s password=%s connect_timeout=0%s",
		 host, port, dbName, login, pwd,
		 data->modeflags == SQLMF_SSL ? " sslmode=require" : "");
	}

	spec->handle = PQconnectdb(coninfo);

	if (PQstatus(spec->handle) == CONNECTION_BAD) {
		free(coninfo);
		return (SQL_ERROR);
	}
	free(coninfo);

	return (SQL_SUCCESS);
}

int
pgsql_prepare(struct sqldata * data, const char *buf)
{
	struct pgsql_data      *spec;

	spec = &data->spec.pgsql;

	memset(spec->qbuff, 0, sizeof spec->qbuff);
	memmove(spec->qbuff, buf, strlen(buf));

	return (SQL_SUCCESS);
}

int
pgsql_fetchhead(struct sqldata * data, struct sql_tuple *tuple)
{
	struct pgsql_data 	*spec;
	struct sql_tuple_head	*head;
	int             	c;

	spec = &data->spec.pgsql;

	if (tuple == NULL)
		return (SQL_ERROR);

	for (c = 0; c < data->cols; ++c) {
		head = calloc(1, sizeof(struct sql_tuple_head));
		if (head == NULL) break;

		pl_add(&tuple->head, head);
		if (tuple->head.rc != LST_ESUCCESS) {
			free(head);
			break;
		}

		head->name = strnalloc(PQfname(spec->result, c), 0);
		if (head->name == NULL) break;

		head->vtype = PQftype(spec->result, c);
		if (head->vtype == NULL) break;

		head->dsize = PQfsize(spec->result, c);
		if (head->dsize == NULL) break;
	}

        if (c < data->cols) { 
	    pl_clear(&tuple->head);
	    return SQL_ERROR;
	}

	return SQL_SUCCESS;
}

int
pgsql_fetch(struct sqldata * data, struct sql_tuple *tuple)
{

	struct pgsql_data 	*spec;
	int             	c;
        int			r;
        int			rfrom;
        int			rto;
	char           		*slice;

	spec = &data->spec.pgsql;

	if (tuple == NULL)
		return (SQL_ERROR);

	rfrom = spec->nextrow;
	if (rfrom == 0 && (data->count == SQL_ALLROWS || data->count > data->rows)) 
	    data->count = data->rows;

	if (rfrom >= data->count) return SQL_SUCCESS;   /* return empty structure */

	rto = rfrom + data->blksz;
	rto = MIN(rto, data->count);

	for (r = rfrom; r < rto; ++r) {
	    for (c = 0; c < data->cols; ++c) {
		slice = strnalloc(PQgetvalue(spec->result, r, c), 0);
		if (slice == NULL) break;

		pl_add(&tuple->data, slice);
		if (tuple->data.rc != LST_ESUCCESS) {
		    free(slice);
		break;
		}
	    }
	    if (c < data->cols) break;
	}
	if (r >= rto) {
	    spec->nextrow = rto;
            return (SQL_SUCCESS);
        }

	pl_clear(&tuple->data);
	return (SQL_ERROR);
}

int
pgsql_execute(struct sqldata * data)
{

	struct pgsql_data *spec;

	spec = &data->spec.pgsql;

	spec->result = PQexec(spec->handle, spec->qbuff);
	switch (PQresultStatus(spec->result)) {
	case PGRES_FATAL_ERROR:
	case PGRES_NONFATAL_ERROR:
	case PGRES_EMPTY_QUERY:
	case PGRES_BAD_RESPONSE:
	case PGRES_COMMAND_OK:
	case PGRES_COPY_IN:
		return (SQL_ERROR);
	case PGRES_TUPLES_OK:
		data->rows    = PQntuples(spec->result);
		data->cols    = PQnfields(spec->result);
		data->count   = data->rows;
		spec->nextrow = 0;
	case PGRES_COPY_OUT:
		return (SQL_SUCCESS);
	}

	return (SQL_SUCCESS);
}

int
pgsql_run(struct sqldata * data)
{
	struct pgsql_data *spec;

	spec = &data->spec.pgsql;

	spec->result = PQexec(spec->handle, spec->qbuff);
	switch (PQresultStatus(spec->result)) {
	case PGRES_FATAL_ERROR:
	case PGRES_NONFATAL_ERROR:
	case PGRES_EMPTY_QUERY:
	case PGRES_BAD_RESPONSE:
	case PGRES_COPY_OUT:
	case PGRES_TUPLES_OK:
		return (SQL_ERROR);
	case PGRES_COMMAND_OK:
	case PGRES_COPY_IN:
		return (SQL_SUCCESS);
	}

	return (SQL_SUCCESS);
}

int
pgsql_finish(struct sqldata * data)
{
	struct pgsql_data *spec;

	spec = &data->spec.pgsql;

	PQclear(spec->result);
	return (SQL_SUCCESS);
}

int
pgsql_close(struct sqldata * data)
{

	struct pgsql_data *spec;

	spec = &data->spec.pgsql;

	PQfinish(spec->handle);
	return (SQL_SUCCESS);
}

int
pgsql_strescape(char * dst, const char * src, int len)
{
   return PQescapeString(dst, src, len-1);
}


#endif				/* __PGSQL */
