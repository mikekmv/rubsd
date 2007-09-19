/* $oganer: lists.h,v 1.1 2004/12/01 15:50:56 shadow Exp $ */

/*
 * Copyright (c) 2002, 2003, 2004 Maxim Tsyplakov <tm@oganer.net>
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

#ifndef _LISTS_H
#define _LISTS_H

#define	LST_DEF_DELTA	(1)
#define	LST_MAX_DELTA	(2 << 17)

/* LIST ERROR CODES */

#define	LST_ESUCCESS	(0)
#define LST_SUCCESS	(LST_ESUCCESS)
#define	LST_EMEMORY	(1)
#define LST_EINVARG	(2)
#define LST_ENOTFOUND	(3)

#define LEFT		(0)
#define RIGHT		(1)
#define	ASCEND		(0)
#define DESCEND		(1)

struct pl;

struct pl {
	int             count;
	int             size;
	int             delta;
	unsigned int    rc;
	void          **ptr;
	void            (*dealloc) (void *);
};

struct pl      *pl_init(struct pl *, int, void (*) (void *));

#define pl_get(L, I) ((L)->ptr[I])

int             pl_add(struct pl *, void *);
int             pl_insert(struct pl *, void *, int);
int 		pl_sinsert(struct pl *, void *,
	   		int (*) (struct pl *, void *, int));
int             pl_delete(struct pl *, int);
int             pl_exchange(struct pl *, int, int);
void           *pl_replace(struct pl *, void *, int);
void            pl_hsort(struct pl *, int (*) (void *, void *));
void           *pl_lsearch(struct pl *, void *, int *,
                   int (*) (void *, void *));
void           *pl_bsearch(struct pl *, void *, int *,
                   int (*) (void *, void *));
void           *pl_fentry(struct pl *, void *, int *,
                   int (*) (void *, void *), int (*) (void *, void *));
void           *pl_lentry(struct pl *, void *, int *,
                   int (*) (void *, void *), int (*) (void *, void *));
void           *pl_isearch(struct pl *, void *, int *,
                   int (*) (struct pl *, void *, int, int),
                   int (*) (void *, void *));

void            pl_clear(struct pl *);
struct pl      *pl_free(struct pl *);
void            pl_pack(struct pl *);

/* default functions */
void            pl_def_dealloc(void *);
int             pl_def_compare(void *, void *);

int		pl_str_explode(struct pl *, char *, const char *);

#endif				/* _LISTS_H */
