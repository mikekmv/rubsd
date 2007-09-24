/* $oganer: lists.c,v 1.1 2005/05/09 14:47:03 shadow Exp $ */

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

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include "ported.h"
#else
#include <limits.h>
#endif /* _WIN32 */

#include "lists.h"

#ifdef __WATCOMC__
#pragma intrinsic(pl_get);
#endif				/* __WATCOMC__ */

__inline static char *next_tok(char *[], const char *);

void
pl_def_dealloc(void *ptr)
{
	free(ptr);
}

int
pl_def_compare(void *ptr, void *key)
{
	return (ptr < key ? -1 : ptr > key ? 1 : 0);
}

struct pl      *
pl_init(struct pl * list, int delta, void (*dealloc) (void *))
{
	struct pl      *lst;

	lst = list == NULL ? malloc(sizeof(struct pl)) : list;

	if (lst != NULL) {
		memset(lst, 0, sizeof(struct pl));
		lst->delta = delta <= 0 || delta > LST_MAX_DELTA
			? LST_DEF_DELTA : delta;
		if (dealloc != NULL)
			lst->dealloc = dealloc;
	}
	return (lst);
}

int
pl_add(struct pl * lst, void *item)
{
	void          **ptr;

	if (lst->count == lst->size) {
		ptr = realloc(lst->ptr,
			      (lst->size + lst->delta) * sizeof(void *));
		if (ptr == NULL) {
			lst->rc = LST_EMEMORY;
			return (-1);
		}
		lst->ptr = ptr;
		lst->size += lst->delta;
	}
	lst->ptr[lst->count++] = item;
	lst->rc = LST_ESUCCESS;
	return (lst->count - 1);
}

int
pl_insert(struct pl * lst, void *item, int index)
{
	void          **ptr;
	int             i;

	if (index < 0 || index > lst->count) {
		lst->rc = LST_EINVARG;
		return (-1);
	}
	if (lst->count == lst->size) {
		ptr = realloc(lst->ptr, (lst->size + lst->delta) * sizeof(void *));
		if (ptr == NULL) {
			lst->rc = LST_EMEMORY;
			return (-1);
		}
		lst->size += lst->delta;
		lst->ptr = ptr;
	}
	for (i = lst->count; i >= index; --i)
		lst->ptr[i] = lst->ptr[i - 1];

	lst->ptr[index] = item;
	++lst->count;
	lst->rc = LST_ESUCCESS;
	return (index);
}

/* sorted insertion */
int
pl_sinsert(struct pl * lst, void *item,
	   int (*compare) (struct pl *, void *, int))
{
	int             l, u, m, rc;

	if (lst->count == 0)
		return (pl_add(lst, item));

	l = 0;
	u = lst->count - 1;

	for (;;) {
		m = (l + u) >> 1;
		rc = compare(lst, item, m);
		if (rc == -1)
			u = m - 1;
		else {
			if (rc == 1) {
				l = m + 1;
			} else {
				lst->rc = LST_ESUCCESS;
				return (pl_insert(lst, item, m));
			}
		}

		if (l > u) {
			lst->rc = LST_ESUCCESS;
			return (pl_add(lst, item));
		}
	}
}

int
pl_delete(struct pl * lst, int index)
{
	void           *ptr;

	if (index >= 0 && index < lst->count) {
		ptr = lst->ptr[index];

		if (index < --lst->count)
			memmove(&lst->ptr[index], &lst->ptr[index + 1],
				(lst->count - index) * sizeof(void *));

		if (ptr != NULL && lst->dealloc != NULL)
			lst->dealloc(ptr);
		return (lst->rc = LST_ESUCCESS);
	}
	return (lst->rc = LST_EINVARG);
}

int
pl_exchange(struct pl * lst, int i0, int i1)
{
	void           *tmp;

	if (i0 >= 0 && i1 >= 0 && i0 < lst->count && i1 < lst->count) {
		if (i0 != i1) {
			tmp = lst->ptr[0];
			lst->ptr[0] = lst->ptr[1];
			lst->ptr[1] = tmp;
		}
		return (lst->rc = LST_ESUCCESS);
	}
	return (lst->rc = LST_EINVARG);
}

void           *
pl_replace(struct pl * lst, void *ptr, int index)
{
	void           *p;

	if (index >= 0 && index < lst->count) {
		p = lst->ptr[index];
		lst->ptr[index] = ptr;
		lst->rc = LST_ESUCCESS;
		return (p);
	}
	lst->rc = LST_EINVARG;
	return (NULL);
}

/* heap sorting */
void
pl_hsort(struct pl * lst, int (*compare) (void *, void *))
{
	int             i, j, j2, k, n;
	void           *tmp;

	n = lst->count;
	/*
	 * Outerloop: considers all nodes, starting at the last one and
	 * working back to the root.
	 */
	for (k = (n >> 1) - 1; k >= 0; k--) {
		/*
		 * k points to the node under evaluation. j2+1 and j2+2 point
		 * at the children of k.
		 */
		tmp = lst->ptr[k];

		/*
		 * Innerloop: for each changed node, recursively check the
		 * child node which was responsible for the change.
		 */
		for (j = k; (j << 1) <= n - 2; j = i) {
			/* find the largest child of node k. */
			j2 = j << 1;		
			if (j2 + 2 > n - 1)
				/* only one child */
				i = j2 + 1;
			else
				i = compare(lst->ptr[j2 + 1],
					    lst->ptr[j2 + 2])
					? j2 + 2 : j2 + 1;

			/* i now points to the child with the highest value. */
			if (compare(tmp, lst->ptr[i]))
				/* promote child */
				lst->ptr[j] = lst->ptr[i];
			else
				break;
		}
		lst->ptr[j] = tmp;
	}

	/* remove roots continuously */
	for (k = n - 1; k > 0; k--) {
		/*
		 * k points to the (sorted) back of the array. switch the
		 * root with the element at the back.
		 */
		tmp = lst->ptr[k];
		lst->ptr[k] = lst->ptr[0];

		/* make the array into a heap again. */
		for (j = 0; (j << 1) <= k - 2; j = i) {
			j2 = j << 1;
			if (j2 + 2 > k - 1)	/* only one child */
				i = j2 + 1;
			else
				i = compare(lst->ptr[j2 + 1],
					    lst->ptr[j2 + 2])
					? j2 + 2 : j2 + 1;

			/* i now points to the child with the highest value. */
			if (compare(tmp, lst->ptr[i]))
				/* promote child */
				lst->ptr[j] = lst->ptr[i];	
			else
				break;
		}
		lst->ptr[j] = tmp;
	}
	lst->rc = LST_ESUCCESS;
}

/* linear search */
void           *
pl_lsearch(struct pl * lst, void *key, int *index,
	   int (*compare) (void *, void *))
{
	int             i;

	if (index != NULL)
		*index = -1;

	if (lst->count > 0) {
		for (i = 0; i < lst->count; ++i) {
			if (compare(lst->ptr[i], key)) {
				if (index != NULL)
					*index = i;
				lst->rc = LST_ESUCCESS;
				return (lst->ptr[i]);
			}
		}
	}
	lst->rc = LST_ENOTFOUND;
	return (NULL);
}

/* binary search */
void           *
pl_bsearch(struct pl * lst, void *key, int *idx,
	   int (*compare) (void *, void *))
{
	int             l, u, m, rc;

	if (lst->count < 1)
		goto exit;

	l = 0;
	u = lst->count - 1;

	for (;;) {
		m = (l + u) >> 1;
		rc = compare(lst->ptr[m], key);
		if (rc < 0)
			u = m - 1;
		else {
			if (rc > 0) {
				l = m + 1;
			} else {
				if (idx != NULL)
					*idx = m;
				lst->rc = LST_ESUCCESS;
				return (lst->ptr[m]);
			}
		}
		if (l > u) {
	exit:
			if (idx != NULL)
				*idx = -1;
			lst->rc = LST_ENOTFOUND;
			return (NULL);
		}
	}
}


/* interpolation search */
void           *
pl_isearch(struct pl * lst, void *key, int *idx,
	   int (*calc) (struct pl *, void *, int, int),
	   int (*compare) (void *, void *))
{
	int             l, u, m, rc;

	if (lst->count < 1)
		goto exit;
	l = 0;
	u = lst->count - 1;

	for (;;) {
		m = calc(lst, key, l, u);
		rc = compare(lst->ptr[m], key);
		if (rc == -1) {
			u = m - 1;
		} else {
			if (rc == 1) {
				l = m + 1;
			} else {
				if (idx != NULL)
					*idx = m;
				lst->rc = LST_ESUCCESS;
				return (lst->ptr[m]);
			}
		}
		if (l > u) {
	exit:
			if (idx != NULL)
				*idx = -1;
			lst->rc = LST_ENOTFOUND;
			return (NULL);
		}
	}
}

/* find first entry of key */
void           *
pl_fentry(struct pl * lst, void *key, int *idx,
	  int (*bcomp) (void *, void *), int (*lcomp) (void *, void *))
{
	void           *res;

	res = pl_bsearch(lst, key, idx, bcomp);

	if (*idx < 0) {
		lst->rc = LST_ENOTFOUND;
		return (NULL);
	}
	lst->rc = LST_ESUCCESS;
	for (; *idx > -1; --*idx) {
		if (!lcomp(lst->ptr[*idx], key))
			return (lst->ptr[++*idx]);
	}

	return (lst->ptr[++*idx]);
}

/* find last entry of key */
void           *
pl_lentry(struct pl * lst, void *key, int *idx,
	  int (*bcomp) (void *, void *), int (*lcomp) (void *, void *))
{
	void           *res;

	res = pl_bsearch(lst, key, idx, bcomp);

	if (*idx < 0) {
		lst->rc = LST_ENOTFOUND;
		return (NULL);
	}
	lst->rc = LST_ESUCCESS;
	for (; *idx < lst->count; ++*idx)
		if (!lcomp(lst->ptr[*idx], key))
			return (lst->ptr[--*idx]);

	return (lst->ptr[--*idx]);
}

void
pl_clear(struct pl * lst)
{
	int             i;

	if (lst->dealloc != NULL)
		for (i = 0; i < lst->count; ++i)
			lst->dealloc(lst->ptr[i]);
	lst->count = 0;
	lst->rc = LST_ESUCCESS;
}

struct pl      *
pl_free(struct pl * lst)
{
	pl_clear(lst);
	free(lst->ptr);
	free(lst);
	lst->rc = LST_ESUCCESS;
	return (NULL);
}

void
pl_pack(struct pl * lst)
{
	void          **ptr;

	if (lst->count > 0 && lst->count < lst->size) {
		ptr = realloc(lst->ptr, lst->count * sizeof(void *));
		if (ptr != NULL) {
			lst->ptr = ptr;
			lst->size = lst->count;
		}
	}
	lst->rc = LST_ESUCCESS;
}

/*
 * split string into tokens, non-destructive version
 * returns: -1 on error
 * tokens count on success, all parameters shouldn't be NULL
 * NOTE: pl_str_explode() do not deallocates from list already allocated chunks
 */
int
pl_str_explode(struct pl * args, char *str, const char *delim)
{
	char           *tok, *token;
	char           *ptr;

	if (args == NULL || delim == NULL || str == NULL)
		return (-1);
	ptr = strdup(str);
	if (ptr == NULL)
		return (-1);
		
	while ((tok = next_tok(&ptr, delim)) != NULL) {
		token = strdup(tok);
		if (token == NULL) {
			goto error;
		}
		if (pl_add(args, token) == -1)
			goto error;
	}
	free(ptr);
	return (args->count);
error:
	if (ptr != NULL)
		free(ptr);
	if (token != NULL)
		free(token);
	return (-1);
}

static char           *
next_tok(char *ptr[], const char * delim)
{
	char           *str;

	do {
		str = strsep(ptr, delim);
	} while (str != NULL && *str == '\0');

	return (str);
}
