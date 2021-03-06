/*	$RuOBSD: md_creatmsg.c,v 1.2 2004/01/14 05:26:51 form Exp $	*/

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

#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "maildir.h"
#include "md_local.h"

struct md_msg *
md_creatmsg(void)
{
	char hostname[MAXHOSTNAMELEN];
	char file[MAXPATHLEN];
	time_t t;
	pid_t pid;
	int i, fd = 0;

	/* get hostname */
	if (gethostname(hostname, sizeof(hostname)) < 0)
		return (NULL);

	pid = getpid();
	for (i = 0; i < MD_OPEN_RETRY; i++) {
		struct md_msg *m;

		if (fd < 0)
			sleep(MD_RETRY_SLEEP);
		if (snprintf(file, sizeof(file), "%s/%u.%d.%s", MD_PATH_TMP,
		    time(&t), pid, hostname) >= (int)sizeof(file)) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		if ((fd = open(file, O_WRONLY|O_CREAT|O_EXCL,
		    MD_FILE_MODE)) < 0)
			continue;
		if ((m = malloc(sizeof(struct md_msg))) == NULL)
			return (NULL);
		if ((m->mm_file = strdup(file)) == NULL) {
			free(m);
			errno = ENOMEM;
			return (NULL);
		}
		m->mm_fd = fd;
		return (m);
	}
	return (NULL);
}
