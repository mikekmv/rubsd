/*
 * $RuOBSD: ctcp.c,v 1.1.1.1 2006/02/24 17:13:31 form Exp $
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
#include <string.h>
#include <time.h>

#include "ircyka/ircyka.h"


static int
cb_privmsg(int argc, char * const argv[])
{
	struct ircyka_channel *ic;
	char buf[32];
	time_t tm;

	if (argv[0] == NULL || argc < 4 || argv[3][0] != '\001')
		return (0);

	if (argv[2][0] == '#') {
	    if ((ic = channel_find(argv[2])) == NULL || ic->ic_count == 0)
		return (0);
	} else if (charset_strcmp(argv[2], irc_nick) != 0)
		return (0);

	if (strlcpy(buf, argv[3], sizeof(buf)) >= sizeof(buf))
		return (0);

	charset_strtolower(buf);
	if (strcmp(buf, "\001ping\001") == 0) {
		irc_notice(argv[0], "\001PING\001");
		return (1);
	} else if (strcmp(buf, "\001time\001") == 0) {
		tm = time(NULL);
		(void)strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y",
		    localtime(&tm));
		irc_notice(argv[0], "\001TIME %s\001", buf);
		return (1);
	} else if (strcmp(buf, "\001version\001") == 0) {
		irc_notice(argv[0], "\001VERSION %s\001", ircyka_ident);
		return (1);
	}

	return (0);
}


static int
ircyka_module_entry(int func, const char *target, const char *arg)
{
	static struct ircyka_callback *icb;

	switch (func) {
	case IME_LOAD:
		if ((icb = irc_register("PRIVMSG", cb_privmsg)) == NULL)
			return (-1);
		break;
	case IME_UNLOAD:
		cmd_unregister(icb);
		break;
	}
	return (0);
}

IRCYKA_MODULE("ctcp", "IRCyka CTCP handler");
