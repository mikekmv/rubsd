/*	$OpenBSD: pgwrap.c,v 1.2 1999/12/09 02:57:07 form Exp $	*/
/*	$RuOBSD: pgwrap.c,v 1.6 2002/09/05 07:25:16 grange Exp $	*/

/*
 * Copyright (c) 1999 Oleg Safiullin
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

/*
 * usage: pgwrap [-D] [-d datadir] [-l logfile] [-u user] command
 */

#include <sys/types.h>

#include <err.h>
#include <login_cap.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pgwrap.h"

int main(int, char **);
__dead static void usage(void);

int
main(int argc, char *argv[])
{
	struct passwd *pw;
	char *pg_user = DEFAULT_PGUSER;
	char *pg_data = NULL, *pg_log = NULL;
	int ch, daemon_mode = 0;

	while ((ch = getopt(argc, argv, "Dd:l:u:")) != -1)
		switch (ch) {
		case 'D':
			daemon_mode++;
			break;
		case 'd':
			pg_data = optarg;
			break;
		case 'l':
			pg_log = optarg;
			break;
		case 'u':
			pg_user = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argv += optind;
	argc -= optind;
	if (!argc)
		usage();

	if ((pw = getpwnam(pg_user)) == NULL)
		errx(1, "%s: No such user", pg_user);

	unsetenv("PGDATA");
	if (setusercontext(NULL, pw, pw->pw_uid, LOGIN_SETALL) < 0)
		err(1, "setusercontext");
	if (getenv("PGDATA") == NULL && setenv("PGDATA",
	    (pg_data == NULL ? DEFAULT_PGDATA : pg_data), 1) < 0)
		err(1, "setenv");

	if (daemon_mode || pg_log != NULL) {
		if (pg_log != NULL) {
			if (freopen(pg_log, "a", stdout) == NULL ||
			    freopen(pg_log, "a", stderr) == NULL)
				err(1, "freopen %s", pg_log);
		}
		if (daemon(0, (pg_log != NULL)) < 0)
			err(1, "daemon");
	}

	(void)execvp(*argv, argv);
	err(1, "execvp");

	return (0);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-D] [-d datadir] [-l logfile] [-u user] command\n",
	    __progname);
	exit(1);
}
