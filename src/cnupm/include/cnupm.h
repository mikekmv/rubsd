/*	$RuOBSD: cnupm.h,v 1.10 2005/11/22 19:05:25 form Exp $	*/

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

#ifndef __CNUPM_H__
#define __CNUPM_H__

#define CNUPM_VERSION_MAJOR	3
#define CNUPM_VERSION_MINOR	11

#ifndef CNUPM_USER
#define CNUPM_USER		"cnupm"
#endif
#define CNUPM_PIDFILE		"cnupm-%s.pid"
#define CNUPM_DUMPFILE		"cnupm-%s.dump"

#define CNUPM_PIDFILE_CHECK	0
#define CNUPM_PIDFILE_CREATE	1
#define CNUPM_PIDFILE_REMOVE	2

#ifndef NULL
#define NULL			(void *)0
#endif

extern char *__progname;

#include <pwd.h>

#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS		extern "C" {
#define __END_DECLS		}
#else	/* !__cplusplus */
#define __BEGIN_DECLS
#define __END_DECLS
#endif	/* __cplusplus */
#endif	/* __BEGIN_DECLS */

__BEGIN_DECLS
int		cnupm_daemon(int);
u_int		cnupm_family(const char *);
pid_t		cnupm_pidfile(int, const char *, ...);
void		cnupm_progname(char **);
int		cnupm_protocol(const char *);
int		cnupm_restrict(struct passwd *);
void		cnupm_version(int);
u_long		cnupm_ulval(const char *, u_long, u_long);
#ifndef HAVE_ERR
void		err(int, const char *, ...);
void		errx(int, const char *, ...);
void		warn(const char *, ...);
void		warnx(const char *, ...);
#endif
#ifndef HAVE_INET_NTOP
const char	*inet_ntop(int, const void *, char *, size_t);
#endif
#ifndef HAVE_SETPROCTITLE
void		setproctitle(const char *, ...);
#endif
#ifndef HAVE_SNPRINTF
int		snprintf(char *, size_t, const char *, ...);
#endif
#ifndef HAVE_STRLCXX
size_t		strlcat(char *, const char *, size_t);
size_t		strlcpy(char *, const char *, size_t);
#endif
__END_DECLS

#endif	/* __CNUPM_H__ */
