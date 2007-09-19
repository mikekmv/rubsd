/* $oganer: mysql.c,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

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

#if defined(__MYSQL)
#include <sys/param.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lists.h"
#include "misc.h"
#include "myint.h"
#include "sql.h"
#include "internal.h"

struct optb     optb_mysql = {
	mysql_init_,
	mysql_open_,
	mysql_close_,
	mysql_prepare_,
	mysql_execute_,
	mysql_run_,
	mysql_fetch_,
	mysql_finish_
};

int
mysql_init_(struct sqldata * data)
{
	return (SQL_SUCCESS);
}

int
mysql_open_(struct sqldata * data, char *server, char *dbname,
	    char *login, char *password)
{
	struct mysql_data *spec;

	spec = &data->spec.mysql;

	spec->handle = mysql_init(NULL);
	if (spec->handle == NULL)
		return (SQL_ERROR);

	spec->handle = mysql_real_connect(spec->handle, server, login,
					  password, dbname, 0, NULL, 0);

	if (spec->handle == NULL)	/* mysql_error(spec->handle) */
		return (SQL_ERROR);

	return (SQL_SUCCESS);
}

int
mysql_prepare_(struct sqldata * data, const char *buf)
{
	struct mysql_data *spec;

	spec = &data->spec.mysql;
	memset(spec->qbuff, 0, sizeof spec->qbuff);
	memmove(spec->qbuff, buf, strlen(buf));
	return (SQL_SUCCESS);
}

int
mysql_execute_(struct sqldata * data)
{
	struct mysql_data *spec;
	int             rc;

	spec = &data->spec.mysql;
	rc = mysql_real_query(spec->handle, spec->qbuff, strlen(spec->qbuff));
	if (rc)
		return (SQL_ERROR);

	spec->result = mysql_store_result(spec->handle);
	if (spec->result) {
		spec->cols = mysql_num_fields(spec->result);
	} else {
		if (mysql_field_count(spec->handle) == 0) {
			spec->rows = mysql_affected_rows(spec->handle);

		} else {
			return (SQL_ERROR);
		}
	}
	return (SQL_SUCCESS);
}

int
mysql_run_(struct sqldata * data)
{
	return (mysql_execute_(data));
}

int
mysql_fetch_(struct sqldata * data, int count, int blksz)
{
	struct mysql_data *spec;
	struct sql_tuple *tuple;
	struct sql_tuple_head *head;
	int             i;
	MYSQL_FIELD    *field;
	MYSQL_ROW       row;
	char           *slice;

	spec = &data->spec.mysql;
	tuple = calloc(1, sizeof(struct sql_tuple));

	if (tuple == NULL) {
		return (NULL);
	}
	pl_init(&tuple->head, spec->cols, head_free);
	pl_init(&tuple->data, blksz, pl_def_dealloc);
	for (i = 0; i < spec->cols; ++i) {
		head = calloc(1, sizeof(struct sql_tuple_head));
		if (head == NULL)
			goto error;

		pl_add(&tuple->head, head);
		if (tuple->head.rc != LST_ESUCCESS) {
			free(head);
			goto error;
		}
		field = mysql_fetch_field(spec->result);
		if (field == NULL)
			goto error;

		head->name = strnalloc(field->name, 0);
		if (head->name == NULL)
			goto error;
		head->vtype = field->type;
		head->dsize = field->max_length;
	}

	while ((row = mysql_fetch_row(spec->result)) != NULL) {
		for (i = 0; i < spec->cols; ++i) {
			slice = strnalloc(row[i], 0);
			if (slice == NULL)
				goto error;
			pl_add(&tuple->data, slice);
			if (tuple->data.rc != LST_ESUCCESS) {
				free(slice);
				goto error;
			}
		}
	}

	return (SQL_SUCCESS);

error:
	tuple_free(tuple);
	return (SQL_ERROR);
}

int
mysql_finish_(struct sqldata * data)
{
	struct mysql_data *spec;

	spec = &data->spec.mysql;
	mysql_free_result(spec->result);
	return (SQL_SUCCESS);
}

int
mysql_close_(struct sqldata * data)
{
	struct mysql_data *spec;

	spec = &data->spec.mysql;
	mysql_close(spec->handle);
	return (SQL_SUCCESS);
}
#endif				/* __MYSQL */
