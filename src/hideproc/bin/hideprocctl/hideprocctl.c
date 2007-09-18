/*	$RuOBSD$	*/

/*
 * Copyright (c) 2004, 2007 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <sys/ioctl.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "hideproc.h"


static int dflag;
static int eflag;
static int nflag;
static int sflag;
static int vflag;


__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-DENSV] [[!]group\n", __progname);
	exit(EX_USAGE);
}

int
main(int argc, char *const argv[])
{
	struct group *grp;
	u_int32_t flags, oflags, version;
	gid_t gid, ogid;
	int ch;

	while ((ch = getopt(argc, argv, "DENSV")) != -1)
		switch (ch) {
		case 'D':
			dflag++;
			break;
		case 'E':
			eflag++;
			break;
		case 'N':
			nflag++;
			break;
		case 'S':
			sflag++;
			break;
		case 'V':
			vflag++;
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
		errx(EX_CONFIG, "Can't specify both -D and -E");

	if (nflag && sflag)
		errx(EX_CONFIG, "Can't specify both -N and -S");

	if ((dflag || eflag || nflag || sflag || argc != 0) && !vflag)
		ch = open(HIDEPROC_DEV, O_RDWR);
	else
		ch = open(HIDEPROC_DEV, O_RDONLY);
	if (ch < 0)
		err(EX_UNAVAILABLE, "open: %s", HIDEPROC_DEV);

	if (ioctl(ch, HIOCGVERSION, &version) < 0)
		err(EX_UNAVAILABLE, "HIOCGVERSION");

	if (vflag) {
		(void)fprintf(stderr, "HideProc Version %u.%u, device %u.%u\n",
		    HPV_MAJOR(HIDEPROC_VERSION), HPV_MINOR(HIDEPROC_VERSION),
		    HPV_MAJOR(version), HPV_MINOR(version));
		return (EX_OK);
	}

	if (ioctl(ch, HIOCGFLAGS, &oflags) < 0)
		errx(EX_UNAVAILABLE, "HIOCGFLAGS");

	if (ioctl(ch, HIOCGGID, &ogid) < 0)
		err(EX_UNAVAILABLE, "HIOCGGID");

	if (!(dflag || eflag || nflag || sflag || argc != 0)) {
		grp = getgrgid(ogid);

		(void)printf("Device status:      %s\n",
		    oflags & HPF_ENABLE ? "Enabled" : "Disabled");
		(void)printf("System processes:   %s\n",
		    oflags & HPF_SYSTEM ? "Displaying" : "Not displaying");
		if (oflags & HPF_DENY)
			(void)printf("Not displaying for: ");
		else
			(void)printf("Displaying for:     ");

		if (grp == NULL)
			(void)printf("gid %u\n", ogid);
		else
			(void)printf("group %s, gid %u\n", grp->gr_name, ogid);

		return (EX_OK);
	}

	flags = oflags;
	gid = ogid;

	if (dflag)
		flags &= ~HPF_ENABLE;
	else if (eflag)
		flags |= HPF_ENABLE;

	if (nflag)
		flags &= ~HPF_SYSTEM;
	else if (sflag)
		flags |= HPF_SYSTEM;

	if (argc != 0) {
		const char *group = argv[0];

		if (*group == '!') {
			group++;
			flags |= HPF_DENY;
		} else
			flags &= ~HPF_DENY;

		if ((grp = getgrnam(group)) == NULL) {
			const char *errstr;

			gid = strtonum(group, 0, GID_MAX, &errstr);
			if (errstr != NULL)
				errx(EX_CONFIG, "%s: %s", argv[0], errstr);
		} else
			gid = grp->gr_gid;
	}

	if (flags != oflags && ioctl(ch, HIOCSFLAGS, &flags) < 0)
		err(EX_UNAVAILABLE, "HIOCSFLAGS");

	if (gid != ogid && ioctl(ch, HIOCSGID, &gid) < 0)
		err(EX_UNAVAILABLE, "HIOCSGID");

	return (EX_OK);
}
