/*	$RuOBSD$	*/

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
#include <sys/ioctl.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "hproc.h"


int main(int, char **);
__dead static void usage(void);

static int dflag;
static int eflag;
static u_int16_t flags;
static gid_t gid;


int
main(int argc, char **argv)
{
	struct group *gr;
	int ch;

	while ((ch = getopt(argc, argv, "DE")) != -1)
		switch (ch) {
		case 'D':
			dflag = 1;
			break;
		case 'E':
			eflag = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	if (argc > 1)
		usage();

	if (dflag && eflag)
		errx(1, "Conflicting options specified");

	if ((ch = open(HPROC_DEV, O_RDONLY)) < 0)
		err(1, "open: %s", HPROC_DEV);
	if (ioctl(ch, HIOCGFLAGS, &flags) < 0)
		err(1, "ioctl: HIOCGFLAGS");

	if (!dflag && !eflag && !argc) {
		if (ioctl(ch, HIOCGGID, &gid) < 0)
			err(1, "ioctl: HIOCGGID");
		gr = getgrgid(gid);
		printf("Hiding is %s\n",
		    flags & HPF_ENABLED ? "enabled" : "disabled");
		printf("Processes %s be viewed by group %s (%u)\n",
		    flags & HPF_DENYGID ? "can't" : "can",
		    gr == NULL ? "?" : gr->gr_name, gid);
		return (0);
	}

	if (eflag)
		flags |= HPF_ENABLED;
	if (dflag)
		flags &= ~HPF_ENABLED;

	if (argc) {
		optarg = *argv;
		if (optarg[0] == '\0')
			usage();
		if (optarg[0] == '!') {
			flags |= HPF_DENYGID;
			optarg++;
		} else
			flags &= ~HPF_DENYGID;
	} else
		optarg = NULL;

	if (optarg != NULL) {
		if ((gr = getgrnam(optarg)) == NULL) {
			char *ep;
			u_long ulval;

			errno = 0;
			ulval = strtoul(optarg, &ep, 10);
			if (*ep != '\0' ||
			    (errno == ERANGE && ulval == ULONG_MAX))
				errx(1, "%s: No such group", optarg);
			gid = ulval;
		} else
			gid = gr->gr_gid;
		if (ioctl(ch, HIOCSGID, &gid) < 0)
			err(1, "ioctl: HIOCSGID");
	}
	if (ioctl(ch, HIOCSFLAGS, &flags) < 0)
		err(1, "ioctl: HIOCSFLAGS");

	return (0);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-DE] [[!]group]\n", __progname);
	exit(1);
}
