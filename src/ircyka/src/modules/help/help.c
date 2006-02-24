/*
 * $RuOBSD$
 *
 * Copyright (c) 2006 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ircyka/ircyka.h"


#define HELPDIR			"help"


static int
cb_help(int argc, char * const argv[])
{
	char path[MAXPATHLEN], buf[IRCYKA_MAX_STRLEN];
	int error, save_errno;
	FILE *fp;

	(void)strlcpy(buf, argv[3] == NULL ? "ircyka" : argv[3], sizeof(buf));
	charset_strtolower(buf);

	(void)snprintf(path, sizeof(path), "%s/%s/%s.hlp", ircyka_conf_dir,
	    HELPDIR, buf);
	if (access(path, R_OK) < 0) {
		if (errno != ENOENT)
			goto oserr;
		(void)snprintf(path, sizeof(path), "%s/%s/%s.hlp",
		    IRCYKA_DATA_DIR, HELPDIR, buf);
	}

	if ((fp = fopen(path, "r")) == NULL) {
	oserr:	irc_privmsg(argv[4], "HELP -- %s", strerror(errno));
		return (1);
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		size_t len;

		len = strlen(buf);
		while (len > 1 &&
		    (buf[len - 1] == '\r' || buf[len - 1] == '\n'))
			buf[--len] = '\0';
		irc_privmsg(argv[4], "\002%s", buf);
	}

	error = ferror(fp);
	save_errno = errno;
	(void)fclose(fp);
	errno = save_errno;
	if (error)
		goto oserr;

	return (1);
}


static int
ircyka_module_entry(int func, const char *target, const char *arg)
{
	static struct ircyka_callback *icb;

	switch (func) {
	case IME_LOAD:
		if ((icb = cmd_register("HELP", cb_help)) == NULL)
			return (-1);
		break;
	case IME_UNLOAD:
		cmd_unregister(icb);
		break;
	}
	return (0);
}

IRCYKA_MODULE("help", "IRCyka help");
