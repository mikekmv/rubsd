/*	$oganer: misc.h,v 1.1 2005/05/09 14:47:04 shadow Exp $	*/

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
 
#ifndef _MISC_H
#define _MISC_H

#if defined(_WIN32) && defined(__WATCOMC__)
#include <windows.h>
#include "depend.h"
#include "ported.h"
#include "types.h"
#endif

#define CRLF		"\r\n"

/* Is leap year? */
#define ISLEAP(Y)	(!(Y & 3)) && (Y % 100 || !(Y % 400))

/* Signal safe syscall macro */
#define SIGSAFE(R, F)				\
	do {					\
		R = (F);			\
	} while (R == -1 && errno == EINTR)

/* Get signal name by id */
#define SIGNAME(S) ((S) > 0 || (S) < _NSIG ? sys_signame[(S)] : "UNKNOWN")

/* free memory without check */
#if defined(MEM_FREE)
#undef MEM_FREE
#endif
#define MEM_FREE(P)	free(P)

/* free memory if not NULL */
#if defined(MEM_DROP)
#undef MEM_DROP
#endif
#define MEM_DROP(P) do {	\
	if (P != NULL)		\
		free(P);	\
} while (0);

/* free memory if not NULL and set ptr to NULL */
#if defined(MEM_RELEASE)
#undef MEM_RELEASE
#endif
#define MEM_RELEASE(P) do {	\
	if (P != NULL) {	\
		free(P);	\
		P = NULL;	\
	}			\
} while (0);

/* recoding tables offsets */
#define WIN2KOI8	(0)
#define KOI82WIN	(1)

/* Get low/high int from int64_t */
#define DWORD_HIGH(d64)		((d64) >> 32)
#define DWORD_LOW(d64)		((d64) & 0xFFFF)

/* sequencies */
#define SEQ_INIT(SEQ, VAL)	(SEQ = VAL);
#define SEQ_NEXT(SEQ)		(++SEQ)
#define SEQ_PREV(SEQ)		(--SEQ)

char	*strnalloc(const char *, int);
int	strrealloc(char **, const char *);
int	str2long(long *, const char *, int);
int	str2ulong(unsigned long *, const char *, int);
int	str264(u_int64_t *, const char *, int);
void	ltrim(char **);
void	rtrim(char **);
void	strupper(char *);
void	strlower(char *);
char   *recode(char *, int);
int 	get_string_index(char *const[], const char *);
int	check_ymd(int, int, int);
int	check_hms(int, int, int);
int	find_field(char *, char *, int, char **);
int	find_chain(char *, int, char **, char **, int (*)(int));

#endif /* _MISC_H */
