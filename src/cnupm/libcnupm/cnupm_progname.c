/*	$RuOBSD: cnupm_progname.c,v 1.2 2004/04/22 03:17:58 form Exp $	*/

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
#if !defined(HAVE_PROGNAME) || !defined(HAVE_SETPROCTITLE)
#include <string.h>
#endif

#include "cnupm.h"

#ifndef HAVE_PROGNAME
char *__progname;
#endif

void
cnupm_progname(char **argv)
{
#if !defined(HAVE_PROGNAME) || !defined(HAVE_SETPROCTITLE)
	char *p, *cp;
#endif
#ifndef HAVE_SETPROCTITLE
	extern char **cnupm_argv;

	cnupm_argv = argv;
#endif
#if !defined(HAVE_PROGNAME) || !defined(HAVE_SETPROCTITLE)
	for (p = cp = *argv; *p != '\0'; p++)
		if (*p == '/')
			cp = p + 1;
	if ((__progname = strdup(cp)) == NULL)
		__progname = cp;
#endif
}
