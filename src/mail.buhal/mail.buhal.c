/*	$RuOBSD: mail.buhal.c,v 1.1.1.1 2002/12/16 06:12:36 grange Exp $	*/

/*
 * Copyright (c) 2002 Oleg Safiullin <form@pdp11.org.ru>
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
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <login_cap.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int, char **);
static void checkmaildir(const char *);
__dead static void usage(void);

#define MAIL_BUFSIZE		2048
#define MAIL_DIR_MODE		0700
#define MAIL_FILE_MODE		0600
#define MAIL_RETRY		4
#define MAIL_RETRY_SLEEP	2
#define PATH_MAILDIR		"Maildir"
#define PATH_CUR		"cur"
#define PATH_NEW		"new"
#define PATH_TMP		"tmp"

int
main(int argc, char **argv)
{
	struct passwd *pw;
	char hostname[MAXHOSTNAMELEN];
	char maildir[PATH_MAX];
	char tmpfile[PATH_MAX];
	char mailfile[PATH_MAX];
	char buf[MAIL_BUFSIZE];
	int ch, fd;

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
		errx(1, "May only be run by the superuser");

	if ((pw = getpwnam(argv[optind])) == NULL)
		errx(1, "%s: No such user", argv[optind]);

	if (pw->pw_dir == NULL || pw->pw_dir[0] == '\0')
		errx(1, "No home directory for %s", argv[optind]);

	if (setusercontext(NULL, pw, pw->pw_uid, LOGIN_SETALL) < 0)
		err(1, "setusercontext");

	if (gethostname(hostname, sizeof(hostname)) < 0)
		err(1, "gethostname");

	if (snprintf(maildir, sizeof(maildir), "%s/%s", pw->pw_dir,
	    PATH_MAILDIR) >= sizeof(maildir))
		errx(1, "Maildir path too long");

	checkmaildir(maildir);

	for (ch = 0; ch < MAIL_RETRY; ch++) {
		time_t tloc;

		if (snprintf(tmpfile, sizeof(tmpfile), "%s/%u.%u.%s",
		    PATH_TMP, time(&tloc), getpid(),
		    hostname) >= sizeof(tmpfile))
			errx(1, "Temporary file path too long");
		if ((fd = open(tmpfile, O_WRONLY|O_CREAT|O_EXCL,
		    MAIL_FILE_MODE)) < 0) {
			(void)sleep(MAIL_RETRY_SLEEP);
			continue;
		} else
			break;
	}
	if (fd < 0)
		err(1, "open %s", tmpfile);

	while ((ch = read(STDIN_FILENO, buf, sizeof(buf))) != 0)
		if (ch < 0 || write(fd, buf, ch) < 0) {
			int save_errno = errno;

			(void)close(fd);
			(void)unlink(tmpfile);
			errno = save_errno;
			err(1, "read/write");
		}
	if (close(fd) < 0) {
		unlink(tmpfile);
		err(1, "close");
	}

	for (ch = 0; ch < MAIL_RETRY; ch++) {
		time_t tloc;

		if (snprintf(mailfile, sizeof(tmpfile), "%s/%u.%u.%s",
		    PATH_NEW, time(&tloc), getpid(),
		    hostname) >= sizeof(mailfile))
			errx(1, "Message file path too long");
		if ((fd = link(tmpfile, mailfile)) < 0) {
			(void)sleep(MAIL_RETRY_SLEEP);
			continue;
		} else
			break;
	}
	if (fd < 0)
		err(1, "link %s -> %s", tmpfile, mailfile);
	(void)unlink(tmpfile);

	return (0);
}

static void
checkmaildir(const char *path)
{
	static char *subdirs[] = { "", PATH_CUR, PATH_NEW, PATH_TMP };
	int i;

	for (i = 0; i < sizeof(subdirs) / sizeof(*subdirs); i++) {
		char subdir[PATH_MAX];

		if (snprintf(subdir, sizeof(subdir), "%s/%s", path,
		    subdirs[i]) >= sizeof(subdir))
			errx(1, "%s subdirectory path too long", subdirs[i]);
		if (mkdir(subdir, MAIL_DIR_MODE) < 0 && errno != EEXIST)
			err(1, "mkdir %s", subdir);
	}
	if (chdir(path) < 0)
		err(1, "chdir %s", path);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s user\n", __progname);
	exit(1);
}
