/* $oganer: da.h,v 1.1 2005/05/09 14:47:03 shadow Exp $ */

/*
 * Copyright (c) 2002, 2003, 2004 Ilya Kovalenko <shadow@oganer.net>
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

#ifndef _DA_H
#define _DA_H

/* Empty dynamic array. Sucess 0, Error (-1). */
int    da_empty(int * cnt, void * array, int size);
/* Adds new entry to given ind (or end if ind=-1), zero fill, return its pointer. Error NULL. */
void * da_new  (int * cnt, void * array, int size, int ind);
/* Adds new entry to given ind (or end if ind=-1) & copy from src. new member no or (-1) on error. */
int    da_ins	(int * cnt, void * array, int size, int ind, void * src);
/* Removes given entry to dst (if not NULL). Sucess 0, Error (-1). */
int    da_rm   (int * cnt, void * array, int size, int ind, void * dst);
/* Returns given entry pointer. Error NULL. */
void * da_ptr	(int * cnt, void * array, int size, int ind);
/* Copies src to given entry. Sucess 0, Error (-1). */
int    da_put	(int * cnt, void * array, int size, int ind, void * src);
/* Copies given entry to dst. Sucess 0, Error (-1). */
int    da_get (int * cnt, void * array, int size, int ind, void * dst);

#endif /* _DA_H */
