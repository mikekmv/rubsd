/*	$oganer: log.c,v 1.1 2005/05/09 14:47:03 shadow Exp $	*/

/*
 * Copyright (c) 2004 Alexey Dmitriev <alexey@oganer.net>
 * 		      Maxim Tsyplakov <tm@oganer.net>
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

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "misc.h"

#ifdef __WINDOWS__
#include "ported.h"
#endif /* __WINDOWS__ */

static void	loglock_init(struct log *);
static void	loglock_free(struct log *);
static void	loglock(struct log *);
static void	logunlock(struct log *);

#define LOCK_INIT(L)	if (L->flags & LOG_MODELOCK) loglock_init(L)
#define LOCK_FREE(L)	if (L->flags & LOG_MODELOCK) loglock_free(L)
#define LOCK(L)		if (L->flags & LOG_MODELOCK) loglock(L)
#define UNLOCK(L)	if (L->flags & LOG_MODELOCK) logunlock(L)

/*
 *  log_open - open log, if operation sucessfully completes
 *  log_open returns ptr, else returns errno if -1 on internal errors
 *
 */
struct log *
log_open(int type, const char *logname, u_int32_t flags)
{
	struct log	    *log;
	int		    len;

	log = NULL;
	
	log = malloc(sizeof(struct log));
	if (log == NULL)
		return (NULL);
		
	log->type = type;
	log->flags = flags;	
	
	if (logname == NULL)
		return (NULL);
	len = strlen(logname);
	if (len >= sizeof log->filename) {
		free(log);
		return (NULL);
	}
		
	switch (type) {	
	case LOG_TO_STREAM:	
		log->data.s.stream = fopen(logname, "a");
		if (log->data.s.stream == NULL) {
			free(log);
			return (NULL);
		}	
		break;	
	case LOG_TO_FILE:
		log->data.f.fd = open(logname, O_WRONLY | O_APPEND, 0);
		if (log->data.f.fd == -1) {
			free(log);
			return (NULL);
		}
	}
		
	strlcpy(log->filename, logname, len);
	LOCK_INIT(log);
	return (log);
}

/*
 *  log_write returns 0 if the operation is successfully completes
 *  on error we return errno or -1 on internal error
 */

int
log_write(struct log *log, const char *fmt,...)
{
	va_list	vl;
	int rc;
	size_t nb;
	char	buf[1024];		

	if (log == NULL)
		return (0);

	LOCK(log);

	rc = -1;	
	switch (log->type) {
	
	case LOG_TO_STREAM:
		if (log->data.s.stream == NULL)
		/* should be never happens */
			goto exit;
		
		va_start(vl, fmt);
		rc = vfprintf(log->data.s.stream, fmt, vl);
		va_end(vl);
		if (!rc && ferror(log->data.s.stream))
			goto exit;
		
		if (fflush(log->data.s.stream) == EOF) {
			rc = errno;
			goto exit;		
		}
		rc = 0;
		break;
	case LOG_TO_FILE:
	/* 
	 * beware, vsnprintf is async safe under OpenBSD
	 * but we don't known about other unices
	 *
	 */
	 	va_start(vl, fmt);
	 	memset(buf, '\0', sizeof buf);
		rc = vsnprintf(buf, sizeof buf, fmt, vl);
		nb = strlen(buf);
		if (nb)
			SIGSAFE(nb, write(log->data.f.fd, buf, nb));
		va_end(vl);
		if (rc != -1)
			rc = errno;
		break;
	}
exit:	
	UNLOCK(log);
	return (rc);
}

/*
 *  log_close returns 0 if the operation is successfully complete
 */

int
log_close(struct log *log)
{
	int rc;

	if (log == NULL) 
		return (0);

	LOCK_FREE(log);
	switch (log->type) {	
	case LOG_TO_STREAM:
		if (fclose(log->data.s.stream) != EOF)
			return (errno);
		free(log);
		return (0);
		
	case LOG_TO_FILE:
		do {
			rc = close(log->data.f.fd);
		} while (errno == EINTR);
		if (rc == -1)
			return (errno);
	}
        
	return (0);
}

static void
loglock_init(struct log * log)
{
#ifdef __WINDOWS__
	InitializeCriticalSection(&log->cs);
#endif /* __WINDOWS__ */
}

static void
loglock_free(struct log * log)
{
#ifdef __WINDOWS__
	DeleteCriticalSection(&log->cs);
#endif /* __WINDOWS__ */
}

static void
loglock(struct log * log)
{
#ifdef __WINDOWS__
	EnterCriticalSection(&log->cs);	
#endif /* __WINDOWS__ */
#ifdef __unix__ 
	switch (log->type) {
	case LOG_TO_STREAM:
		(void)flock(fileno(log->data.s.stream), LOCK_EX);
		break;
	case LOG_TO_FILE:
		(void)flock(log->data.f.fd, LOCK_EX);
		break;
	}
#endif	
}

static void
logunlock(struct log * log)
{
#ifdef __WINDOWS__
	DeleteCriticalSection(&log->cs);	
#endif /* __WINDOWS__ */
#ifdef __unix__ 
	switch (log->type) {
	case LOG_TO_STREAM:
		(void)flock(fileno(log->data.s.stream), LOCK_UN);
		break;
	case LOG_TO_FILE:
		(void)flock(log->data.f.fd, LOCK_UN);
		break;
	}
#endif	
}
