/*	$RuOBSD: cnupm_restrict.c,v 1.2 2004/04/26 04:06:44 form Exp $	*/

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
#ifdef HAVE_INITGROUPS
#include <grp.h>
#endif
#ifdef HAVE_LOGIN_CAP
#include <login_cap.h>
#endif
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "cnupm.h"

int
cnupm_restrict(struct passwd *pw)
{
	tzset();
	openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);
#ifdef HAVE_LOGIN_CAP
	if (setusercontext(NULL, pw, pw->pw_uid,
	    LOGIN_SETALL & ~LOGIN_SETUSER) < 0)
#else	/* !HAVE_LOGIN_CAP */
#ifdef HAVE_INITGROUPS
	if (initgroups(pw->pw_name, pw->pw_gid) < 0)
#endif	/* HAVE_INITGROUPS */
#endif	/* HAVE_LOGIN_CAP */
		return (-1);
	if (chroot(pw->pw_dir) < 0 || setuid(pw->pw_uid) < 0 || chdir("/") < 0)
		return (-1);
	return (0);
}
