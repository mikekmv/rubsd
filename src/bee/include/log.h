/*	$oganer: log.h,v 1.1 2005/05/09 14:47:03 shadow Exp $	*/

/*
 * Copyright (c) 2004 Alexey Dmitriev <alexey@oganer.net>
 *		      Maxim Tsyplakov <tm@oganer.net>
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

#ifndef _LOG_H 
#define _LOG_H

#if defined(_WIN32) && defined(__WATCOMC__)
#include <windows.h>
#include "ported.h"
#endif

#ifndef _MAX_PATH
#define _MAX_PATH PATH_MAX
#endif /* _MAX_PATH */

#define LOG_TO_STREAM	(0)
#define LOG_TO_FILE	(1)	

struct log_stream {	
	FILE	*stream;	
};

struct log_file {
	int	fd;
};

struct log {
	int		    type;
	char		    filename[_MAX_PATH + 1];
	u_int32_t	    flags;
#define LOG_MODELOCK	(0x01)	
	union {
		struct log_stream s;
		struct log_file	f;
	} data;	
#ifdef _WIN32	
	CRITICAL_SECTION    cs;
#endif /* _WIN32 */	
	int		    ret;
};

struct log	*log_open(int, const char *, u_int32_t);
int		log_write(struct log *, const char *, ...);
int		log_close(struct log *);

#endif				/* _LOG_H */
