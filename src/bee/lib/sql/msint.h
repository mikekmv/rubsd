/* $oganer: msint.h,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

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

#ifndef _MSINT_H
#define _MSINT_H

#include "sql.h"

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

extern struct optb optb_mssql;

#endif /* _MSINT_H */
