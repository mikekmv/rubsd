/* $oganer: sql.c,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

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

#if defined(__MSSQL) || defined(__MYSQL) || defined(__PGSQL)
#include <sys/param.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lists.h"

#ifdef __MSSQL
#include <sqlfront.h>
#include <sybdb.h>

#include "msint.h"
#endif

#ifdef __MYSQL
#include "myint.h"
#endif

#ifdef __PGSQL
#include "pgint.h"
#endif

#include "sql.h"
#include "internal.h"
#include "misc.h"

static struct optb *optb[] = {
#ifdef __MSSQL
	&optb_mssql,
#endif				/* __MSSQL */
#ifdef __MYSQL
	&optb_mysql,
#endif				/* __MYSQL */

#ifdef __PGSQL
	&optb_pgsql
#endif				/* __PGSQL */
};

static struct sql_enum se[] = {
#ifdef __MSSQL
	{SQLTYPE_MSSQL, "MSSQL"},
#endif
#ifdef __MYSQL
	{SQLTYPE_MYSQL, "MYSQL"},
#endif
#ifdef __PGSQL
	{SQLTYPE_PGSQL, "PGSQL"},
#endif
	{-1, NULL}	/* terminator */
};

int tuple_init(struct sql_tuple *, int, int);



struct sqldata *
sql_init(struct sqldata * data, int type)
{
	int f =  0;
	
	if (data == NULL) {
		data = calloc(1, sizeof(struct sqldata));
		if (data == NULL)
			return (NULL);		
		f = 1;
	}

/* return error (NULL) on double initialisation */
	if ((data->stateflags & SQLF_INIT))
		return NULL;               

/* zero fill structure */
        bzero(data, sizeof(*data));
	data->type = type;
        data->rows = (-1);

	if (optb[type]->init(data) != SQL_SUCCESS) {
		return(NULL);
	}

	data->stateflags = SQLF_INIT;
	return (data);
}

int
sql_open(struct sqldata * data, char *server, char *dbname, char *login,
	 char *password)
{
	int 	rc = SQL_ERROR; /* return error on double open */
	char	*ptr;
	char	*hostname;
	int     port = 0;     /* i.e. use default port */
	
	if (!(data->stateflags & SQLF_OPENED)) {

		hostname = calloc(1, SQLD_HOSTNAMESZ);
		if (hostname == NULL) return SQL_ERROR;

/* separate hostname & port number */
                ptr = strchr(server, ':');
		if (ptr != NULL) {
		   ptr++;
		   port = strtol(ptr, NULL, 10);
		   strlcpy(hostname, server, MIN(SQLD_HOSTNAMESZ, 
                               ((u_long)ptr - (u_long)server)));
		}
		else strlcpy(hostname, server, SQLD_HOSTNAMESZ);
		
		rc = optb[data->type]->open(data, hostname, port, dbname, login, password);
		if (rc == SQL_SUCCESS)
			data->stateflags |= SQLF_OPENED;
		free(hostname);
	}

	return (rc);
}

int
sql_close(struct sqldata * data)
{
	int rc = SQL_SUCCESS;   /* ignore double close */

	if (data->stateflags & SQLF_OPENED) {
		rc = optb[data->type]->close(data);
		if (rc == SQL_SUCCESS)
			data->stateflags &= ~SQLF_OPENED;
	}
	return (rc);
}

int
sql_prepare(struct sqldata * data, const char *format, ...)
{
        int		rc;
	char            *buf;
	va_list         valist;

        buf = calloc(1, SQLD_QBUFFSZ);
        if (buf == NULL) return SQL_ERROR;

	if (format != NULL) {
		va_start(valist, format);
		vsnprintf(buf, SQLD_QBUFFSZ, format, valist);
		va_end(valist);
	}
	rc = optb[data->type]->prepare(data, buf);
	free(buf);

	return (rc);
}

int
sql_execute(struct sqldata * data)
{
	return (optb[data->type]->execute(data));
}

int
sql_getrowscount(struct sqldata * data)
{
	return data->rows;
}

int
tuple_init(struct sql_tuple * tuple, int hdelta, int ddelta)
{
	if (tuple == NULL)
		return (SQL_ERROR);

	pl_init(&tuple->head, hdelta, head_free);
	pl_init(&tuple->data, ddelta, pl_def_dealloc);
	
	return SQL_SUCCESS;
}

int
sql_fetchhead(struct sqldata * data, struct sql_tuple ** ptuple)
{
	if (ptuple == NULL) return SQL_ERROR;

	if (*ptuple == NULL) {
	    *ptuple = calloc(1, sizeof(struct sql_tuple));
	    if (*ptuple == NULL) return SQL_ERROR;
	    if (tuple_init(*ptuple, data->cols, SQLD_FETCHBLKSZ) == SQL_ERROR)
		return SQL_ERROR;
	}

	pl_clear(&((*ptuple)->head));

	return (optb[data->type]->fetchhead(data, *ptuple));
}

int
sql_fetch(struct sqldata * data, struct sql_tuple ** ptuple, int count, int blksz)
{
	if (ptuple == NULL) return SQL_ERROR;


	if (count <= 0 && count != SQL_ALLROWS && count != SQL_FETCHNEXT)
		count = SQL_ALLROWS;

	if (blksz <= 0)
		blksz = SQLD_FETCHBLKSZ;

	if (blksz >= count && count != SQL_ALLROWS && count != SQL_FETCHNEXT)
		blksz = count;

	if (*ptuple == NULL) {
	    *ptuple = calloc(1, sizeof(struct sql_tuple));
	    if (*ptuple == NULL) return SQL_ERROR;
	    if (tuple_init(*ptuple, data->cols, blksz) == SQL_ERROR)
		return SQL_ERROR;
	}

	pl_clear(&((*ptuple)->data));

/* store fetch arguments */
	data->count   = count;
	data->blksz   = blksz;

	return (optb[data->type]->fetch(data, *ptuple));
}

int
sql_fetchnext(struct sqldata * data, struct sql_tuple ** ptuple)
{
	if (ptuple == NULL) return SQL_ERROR;

	if (*ptuple == NULL) {
	    *ptuple = calloc(1, sizeof(struct sql_tuple));
	    if (*ptuple == NULL) return SQL_ERROR;
	    if (tuple_init(*ptuple, data->cols, data->blksz) == SQL_ERROR)
		return SQL_ERROR;
	}

	pl_clear(&((*ptuple)->data));

	return (optb[data->type]->fetch(data, *ptuple));
}

int
sql_inittuple(struct sql_tuple * tuple)
{
	return tuple_init(tuple, 0, 0);
}

int
sql_freetuple(struct sql_tuple * tuple)
{
	if (tuple == NULL)
		return (SQL_ERROR);

	pl_clear(&tuple->head);
	pl_clear(&tuple->data);

	return SQL_SUCCESS;
}

int
sql_run(struct sqldata * data)
{
	return (optb[data->type]->run(data));
}

int
sql_finish(struct sqldata * data)
{
	data->rows = (-1);
	return (optb[data->type]->finish(data));
}

int
sql_type(const char * str)
{
	struct sql_enum * tmp;
	
	tmp = se;
	while (tmp->type != NULL)
		if (!strcasecmp(tmp->type, str))
			return (tmp->id);
		else {
			++tmp;
		}
	return (-1);
}

struct sql_tuple * tuple_free(struct sql_tuple * tuple)
{
	if (tuple != NULL) {
		pl_clear(&tuple->head);
		pl_clear(&tuple->data);
		free(tuple);
	}
	return (NULL);
}

void
head_free(void *data)
{
	struct sql_tuple_head *head;

	head = data;
	if (head->name != NULL)
		free(head->name);
	free(head);
}

int
sql_strescape(struct sqldata * data, char * dst, const char * src, int len)
{
   return (optb[data->type]->strescape(dst, src, len));
}

#endif				/* __MSSQL or __MYSQL or __PGSQL */
