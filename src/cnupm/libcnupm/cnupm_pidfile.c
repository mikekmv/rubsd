/*	$RuOBSD$	*/

/*
 * Copyright (c) 2003-2004 Oleg Safiullin <form@pdp-11.org.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/param.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cnupm.h"

pid_t
cnupm_pidfile(int func, const char *fmt, ...)
{
	char buf[MAXPATHLEN], *ep;
	FILE *fp;
	va_list ap;
	int error, save_errno;
	u_long ulval;

	va_start(ap, fmt);
	ulval = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (ulval >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	switch (func) {
	case CNUPM_PIDFILE_CHECK:
		if ((fp = fopen(buf, "r")) == NULL)
			return (errno == ENOENT ? 0 : -1);
		if (fgets(buf, sizeof(buf), fp) == NULL) {
		done:	save_errno = errno;
			error = ferror(fp);
			(void)fclose(fp);
			errno = save_errno;
			return (error ? -1 : 0);
		}
		(void)fclose(fp);
		errno = 0;
		ulval = strtoul(buf, &ep, 10);
		if (*buf == '\0' || *buf == '\n' ||
		    (*ep != '\0' && *ep != '\n') ||
		    (errno == ERANGE && ulval == ULONG_MAX))
			return (0);
		if (kill((pid_t)ulval, 0) == 0 || errno == ESRCH)
			return (0);
		return ((pid_t)ulval);
	case CNUPM_PIDFILE_CREATE:
		if ((fp = fopen(buf, "w")) == NULL)
			return (-1);
		(void)fprintf(fp, "%d\n", getpid());
		goto done;
	case CNUPM_PIDFILE_REMOVE:
		return (unlink(buf));
	}

	errno = EINVAL;
	return (-1);
}
