/*	$RuOBSD: md_mkdir.c,v 1.2 2004/01/14 05:26:51 form Exp $	*/

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
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "maildir.h"
#include "md_local.h"

int
md_mkdir(const char *path)
{
	char maildir[MAXPATHLEN];
	unsigned int i;

	if (snprintf(maildir, sizeof(maildir), "%s/%s", path,
	    _maildir_name) >= (int)sizeof(maildir)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	/* create Maildir (assume ok if already exists) and chdir to */
	if ((mkdir(maildir, MD_DIR_MODE) < 0 && errno != EEXIST) ||
	    chdir(maildir) < 0)
		return (-1);

	/* create subdirectories (assume ok if already exist) */
	for (i = 0; i < sizeof(_maildir_dirs) / sizeof(*_maildir_dirs); i++)
		if (mkdir(_maildir_dirs[i], MD_DIR_MODE) < 0 &&
		    errno != EEXIST)
			return (-1);
	return (0);
}
