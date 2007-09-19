/* $oganer: internal.h,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

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

#ifndef _INTERNAL_H
#define _INTERNAL_H

#if defined(__MSSQL) || defined(__MYSQL) || defined(__PGSQL)

#include "sql.h"

struct optb {
	int             (*init)     (struct sqldata *);
	int             (*open)     (struct sqldata *, char *, int, char *, char *, char *);
	int             (*close)    (struct sqldata *);
	int             (*prepare)  (struct sqldata *, const char *);
	int             (*execute)  (struct sqldata *);
	int             (*run)      (struct sqldata *);
	int             (*fetchhead)(struct sqldata *, struct sql_tuple *);
	int             (*fetch)    (struct sqldata *, struct sql_tuple *);
	int             (*finish)   (struct sqldata *);
        int             (*strescape) (char *, const char *, int);
};

struct sql_enum {
	int 	id;
	char	*type;
};

struct sql_tuple *tuple_free(struct sql_tuple *);
void            head_free(void *);

extern char    *__progname;

#endif				/* __MSSQL or __MYSQL or __PGSQL */

#endif				/* _INTERNAL_H */
