/* $oganer: pgint.h,v 1.1 2005/05/09 14:47:03 shadow Exp $	 */

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
 
#ifndef _PGINT_H
#define _PGINT_H

#include "sql.h"

int             pgsql_init(struct sqldata *);
int             pgsql_open(struct sqldata *, char *, char *, char *, char *);
int             pgsql_prepare(struct sqldata *, const char *);
int             pgsql_fetch(struct sqldata *, struct sql_tuple *, int, int);
int             pgsql_execute(struct sqldata *);
int             pgsql_run(struct sqldata *);
int             pgsql_finish(struct sqldata *);
int             pgsql_close(struct sqldata *);

extern struct optb optb_pgsql;
#endif				/* _PGINT_H */
