/*	$OpenBSD: pgwrap.c,v 1.2 1999/12/09 02:57:07 form Exp $	*/
/*	$RuOBSD: pgwrap.c,v 1.4 2002/09/03 11:00:40 form Exp $	*/

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
 * Usage: pgwrap [-n] [-o file] cmd [arg ...]
 *
 * Execute PostgreSQL program with proper environment and uid/gid.
 * You must have root privilegies to execute this program.
 *
 * OPTIONS:
 *	-D	- Detach from controlling terminal. This also redirects
 *		  stdin to /dev/null.
 * 	-n	- Do not add PostgreSQL binary prefix to cmd.
 *	-o file	- Redirect stdout & stderr to file (write permissions for
 *		  PostgreSQL user required).
 *	-p file	- Save process ID to file.
 */

#include <sys/types.h>
#include <err.h>
#include <libgen.h>
#include <limits.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <login_cap.h>

#include "pgwrap.h"

extern char **environ;

int main(int, char **);
static void usage(void);
static int setupenv(struct passwd *);

int
main(int argc, char **argv)
{
	struct passwd *pw;
	char prog[PATH_MAX], *file = NULL, *pid_file = NULL;
	int ch, nflag = 0, Dflag = 0;

	if (getuid())
		errx(1, "must be root to run this program");

	while ((ch = getopt(argc, argv, "Dno:p:")) != -1) {
		switch (ch) {
		case 'D':
			Dflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'o':
			file = optarg;
			break;
		case 'p':
			pid_file = optarg;
			break;
		default:
			usage();
			break;
		}
	}
	if (!(argc -= optind))
		usage();
	argv += optind;

	if ((pw = getpwnam(PGUSER)) == NULL)
		errx(1, "%s: no such user", PGUSER);

	if (setusercontext(NULL, pw, pw->pw_uid, LOGIN_SETALL) < 0)
		err(1, "setusercontext");

	if (setupenv(pw))
		errx(1, "can't initizlize environment");

	if (file != NULL) {
		if (freopen(file, "a", stdout) == NULL)
			err(1, "can't open `%s'", file);
		(void) freopen(file, "a", stderr);
	}

	if (Dflag) {
		if (freopen("/dev/null", "r", stdin) == NULL)
			err(1, "can't open /dev/null");
		if (daemon(0, 1) < 0)
			err(1, "daemon");
	}

	if (pid_file != NULL) {
		FILE *fp;

		if ((fp = fopen(pid_file, "w")) == NULL)
			warn("can't open PID file");
		else {
			(void) fprintf(fp, "%d\n", getpid());
			(void) fclose(fp);
		}
	}

	if (!nflag) {
		(void) snprintf(prog, sizeof(prog), "%s/%s", PGBIN, *argv);
		*argv = prog;
		(void) execve(*argv, argv, environ);
		err(1, "execve");
	} else {
		(void) execvp(*argv, argv);
		err(1, "execvp");
	}

	/* not reached */
	return (0);
}

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr,
	    "usage: %s [-D] [-n] [-o file] [-p pidfile] cmd [arg ...]\n", __progname);
	exit(1);
}

static int
setupenv(struct passwd *pw)
{
	int rval = 0;

	rval += setenv("PGLIB", PGLIB, 0);
	rval += setenv("PGDATA", PGDATA, 0);

	rval += setenv("PATH", PGPATH, 1);
	rval += setenv("SHELL", PGSHELL, 1);
	rval += setenv("USER", pw->pw_name, 1);
	rval += setenv("LOGNAME", pw->pw_name, 1);
	rval += setenv("HOME", pw->pw_dir, 1);

	return (rval);
}
