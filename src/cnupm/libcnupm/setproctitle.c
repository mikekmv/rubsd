/*	$RuOBSD: setproctitle.c,v 1.1 2004/04/19 12:53:43 form Exp $	*/

/*
 * Copyright (c) 2004 Oleg Safiullin <form@pdp-11.org.ru>
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

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "cnupm.h"

char **cnupm_argv;

void
setproctitle(const char *fmt, ...)
{
	extern char **environ;
	extern char *__progname;
	char buf[64];
	static size_t cnupm_argv_size;
	size_t len;
	va_list ap;

	if (cnupm_argv_size == 0) {
		char **p;

		p = (int)(*environ - *cnupm_argv) > 0 ? environ : cnupm_argv;
		for (; *p != NULL; p++)
			;
		cnupm_argv_size = (size_t)(*--p - *cnupm_argv);
		cnupm_argv_size += strlen(*p) + 1;
		cnupm_argv[1] = *environ = NULL;
	}

	(void)snprintf(buf, sizeof(buf), "%s: ", __progname);
	len = strlen(buf);
	va_start(ap, fmt);
	if (len < sizeof(buf))
		(void)vsnprintf(buf + len, sizeof(buf) - len, fmt, ap);
	bzero(*cnupm_argv, cnupm_argv_size);
	(void)strlcpy(*cnupm_argv, buf, cnupm_argv_size);
	va_end(ap);
}
