/*	$RuOBSD: mail.buhal.c,v 1.4 2002/12/19 04:37:13 form Exp $	*/

/*
 * Copyright (c) 2002 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <sys/time.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <maildir.h>

#define MD_MSGBUF_SIZE	4096

int main(int, char **);
static void alrm_handler(int);
__dead static void usage(void);

int
main(int argc, char **argv)
{
	struct itimerval it;
	struct passwd *pw;
	struct md_msg *m;
	char buf[MD_MSGBUF_SIZE];
	ssize_t len;
	int ch;

	while ((ch = getopt(argc, argv, "lLdf:r:H")) != -1)
		switch (ch) {
		case 'l':	/* compatibility with mail.local */
		case 'L':	/* compatibility with mail.local */
		case 'd':	/* compatibility with mail.local */
		case 'f':	/* compatibility with mail.local */
		case 'r':	/* compatibility with mail.local */
		case 'H':	/* compatibility with mail.local */
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	if (argc - optind != 1)
		usage();

	if (geteuid())
		errx(EX_NOPERM, "May only be run by the superuser");

	if ((pw = getpwnam(argv[optind])) == NULL)
		errx(EX_NOUSER, "%s: No such user", argv[optind]);

	/* paranoia */
	if (pw->pw_passwd != NULL && pw->pw_passwd[0] != '\0')
		bzero(pw->pw_passwd, strlen(pw->pw_passwd));

	if (pw->pw_dir == NULL || pw->pw_dir[0] == '\0')
		errx(EX_UNAVAILABLE, "No home directory for %s", argv[optind]);

	if (initgroups(pw->pw_name, pw->pw_gid) < 0)
		err(EX_UNAVAILABLE, "initgroups");

	if (seteuid(pw->pw_uid) < 0 || setuid(pw->pw_uid) < 0)
		err(EX_UNAVAILABLE, "setuid");

	if (md_mkdir(pw->pw_dir) < 0)
		err(EX_CANTCREAT, "md_mkdir");

	if (signal(SIGALRM, alrm_handler) == SIG_ERR)
		warnx("signal");
	else {
		bzero(&it, sizeof(it));
		it.it_value.tv_sec = 86400;
		if (setitimer(ITIMER_REAL, &it, NULL) < 0)
			warn("setitimer");
	}

	if ((m = md_creatmsg()) == NULL)
		err(EX_CANTCREAT, "md_creatmsg");

	ch = 0;
	while ((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		ch++;
		if (md_writemsg(m, buf, len) < 0) {
			int save_errno;

			save_errno = errno;
			(void)md_purgemsg(m);
			errno = save_errno;
			err(EX_IOERR, "md_writemsg");
		}
	}
	if (ch == 0) {
		(void)md_purgemsg(m);
		errx(EX_NOINPUT, "No input data");
	}

	if (md_closemsg(m) < 0)
		err(EX_IOERR, "md_closemsg");

	return (0);
}

static void
alrm_handler(int signo)
{
	errx(EX_TEMPFAIL, "Couldn't deliver message within 24 hours");
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s user\n", __progname);
	exit(EX_USAGE);
}
