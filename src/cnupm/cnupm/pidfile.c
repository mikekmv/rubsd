/*	$RuOBSD$	*/

/*
 * Copyright (c) 2003 Oleg Safiullin <form@pdp11.org.ru>
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cnupm.h"

int
cnupm_pidfile(const char *interface, int func)
{
	char file[MAXPATHLEN];
	unsigned long ulval;
	char *endptr;
	FILE *fp;

	if (snprintf(file, sizeof(file), CNUPM_PIDFILE, interface) >=
	    sizeof(file)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	switch (func) {
	case CNUPM_PIDFILE_CHECK:
		if ((fp = fopen(file, "r")) == NULL)
			return (errno == ENOENT ? 0 : -1);
		endptr = fgets(file, sizeof(file), fp);
		(void)fclose(fp);
		if (endptr == NULL)
			return (0);
		errno = 0;
		ulval = strtoul(file, &endptr, 10);
		if (file[0] == '\0' || *endptr != '\n' || errno == ERANGE)
			return (0);
		return (kill((pid_t)ulval, 0) < 0 ? 0 : 1);
	case CNUPM_PIDFILE_CREATE:
		if ((fp = fopen(file, "w+")) == NULL)
			return (-1);
		(void)fprintf(fp, "%u\n", getpid());
		(void)fclose(fp);
		break;
	case CNUPM_PIDFILE_REMOVE:
		return (unlink(file));
	}

	return (0);
}
