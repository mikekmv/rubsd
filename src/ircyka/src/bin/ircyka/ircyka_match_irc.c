/*
 * $RuOBSD$
 *
 * Copyright (c) 2005-2006 Oleg Safiullin <form@pdp-11.org.ru>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "ircyka/ircyka.h"
#include "ircyka/acldb.h"


struct ircyka_handler_tree ircyka_irc_handlers;


static int
cb_kick(int argc, char * const argv[])
{
	struct ircyka_channel *ic;

	if (argv[0] == NULL || argc < 4)
		return (0);

	if (charset_strcmp(argv[3], irc_nick) == 0 &&
	    (ic = channel_find(argv[2])) != NULL) {
		if (irc_bitch)
			(void)irc_printf(":%s MODE %s -o %s", irc_nick,
			    argv[2], argv[0]);
		(void)irc_printf(":%s SJOIN %u %s +%s :@%s", irc_hostname,
		    ic->ic_time, ic->ic_channel, channel_aflags(ic->ic_flags),
		    irc_nick);
		if (irc_bitchmsg != NULL)
			(void)irc_privmsg(argv[2], "%s", irc_bitchmsg);
	}

	return (1);
}

static int
cb_kill(int argc, char * const argv[])
{
	if (argc > 2) {
		if (charset_strcmp(argv[2], irc_nick) == 0)
			ircyka_die++;
		return (1);
	}
	return (0);
}

static int
cb_mode(int argc, char * const argv[])
{
	struct ircyka_channel *ic;

	if (argv[0] == NULL || argc < 4 || argv[2][0] != '#')
		return (0);

	if (argc == 4 || (argc == 5 && argv[4][0] == '\0')) {
		if ((ic = channel_find(argv[2])) != NULL) {
			ic->ic_flags = channel_flags(argv[3], ic->ic_flags);
			return (1);
		}
		return (0);
	}

	if (argc > 4) {
		size_t i, len;
		int ap, minus = 0;

		for (i = 0, ap = 4, len = strlen(argv[3]); i < len; i++) {
			if (argv[3][i] == '+')
				minus = 0;
			else if (argv[3][i] == '-')
				minus = 1;
			else {
				if (minus && argv[3][i] == 'o' &&
				    charset_strcmp(argv[ap], irc_nick) == 0) {
					if (irc_bitch)
						(void)irc_printf(
						    ":%s MODE %s -o %s",
						    irc_nick, argv[2],
						    argv[0]);
					(void)irc_printf(":%s MODE %s +o %s",
					    irc_nick, argv[2], irc_nick);
					if (irc_bitchmsg != NULL)
						(void)irc_privmsg(argv[2],
						    "%s", irc_bitchmsg);
				}
				if (++ap == argc)
					break;
			}
		}
		return (1);
	}

	return (0);
}

static int
cb_nick(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	const char *errstr;
	time_t tm;

	if (argv[0] == NULL && argc > 9 &&
	    (in = nick_create(argv[2], argv[7])) != NULL) {
		tm = strtonum(argv[4], 0, UINT_MAX, &errstr);
		in->in_time = (errstr == NULL) ? tm : time(NULL);
		return (1);
	}

	if (argv[0] != NULL && argc > 2 && (in = nick_find(argv[0])) != NULL &&
	    nick_change(in, argv[2]) == 0)
		return (1);

	return (0);
}

static int
cb_part(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_channel *ic;
	struct ircyka_join *ij;

	if (argc > 2 && argv[0] != NULL && (in = nick_find(argv[0])) != NULL &&
	    (ic = channel_find(argv[2])) != NULL) {
		LIST_FOREACH(ij, &in->in_joins, ij_nentry)
			if (ij->ij_ic == ic) {
				join_destroy(ij);
				break;
			}
		return (1);
	}
	return (0);
}

static int
cb_ping(int argc, char * const argv[])
{
	int i;

	if (argc < 3)
		return (0);

	if (--argc > 2) {
		for (i = 2; i < argc; i++)
			if (strcasecmp(argv[i], irc_hostname) == 0)
				break;
		if (i == argc)
			return (0);
	}

	(void)irc_printf(":%s PONG %s :%s", irc_hostname, irc_hostname,
	    argv[argc]);

	if (ircyka_phase == 0) {
		(void)irc_printf("PING :%s", irc_hostname);
		ircyka_phase++;
	}

	return (1);
}

static int
cb_pong(int argc, char * const argv[])
{
	struct ircyka_channel *ic;
	struct ircyka_nick *in;

	if (argc > 2) {
		if (ircyka_phase == 1) {
			if ((in = nick_find(irc_nick)) != NULL) {
				if (irc_killmsg != NULL)
					(void)irc_printf("KILL %s :%s (%s)",
					    irc_nick, irc_hostname,
					    irc_killmsg);
				else
					(void)irc_printf("KILL %s :%s",
					    irc_nick, irc_hostname);
				nick_destroy(in);
			}
			(void)irc_printf("NICK %s 1 %u +i %s %s %s :%s",
			    irc_nick, time(NULL), irc_user, irc_domain,
			    irc_hostname, irc_name);
			RB_FOREACH(ic, ircyka_channel_tree, &ircyka_channels)
				if ((ic->ic_flags & ICF_PERSIST) ||
				    irc_joinall)
					(void)irc_join(ic, irc_nick);
			ircyka_phase++;
		}
		return (1);
	}
	return (0);
}

static int
cb_privmsg(int argc, char * const argv[])
{
	struct ircyka_channel *ic;
	char *cp, *p;

	if (argv[0] == NULL || argc < 4)
		return (0);

	if (argv[2][0] == '#' && argv[3][0] == '!') {
		if ((ic = channel_find(argv[2])) == NULL || ic->ic_count == 0)
			return (0);
		p = argv[3] + 1;
	} else if (charset_strcmp(argv[2], irc_nick) == 0) {
		p = argv[3];
		if (*p == '!')
			p++;
	} else
		return (0);

	if ((cp = strchr(p, ' ')) != NULL) {
		*cp++ = '\0';
		while (*cp == ' ')
			cp++;
		if (*cp == '\0')
			cp = NULL;
	}

	match_cmd(argv[0], argv[2][0] == '#' ? argv[2] : argv[0], p, cp);
	return (1);
}

static int
cb_quit(int argc, char * const argv[])
{
	struct ircyka_nick *in;

	if (argv[0] != NULL && argc > 1 && (in = nick_find(argv[0])) != NULL) {
		if (in->in_flags & INF_LOGON)
			nick_logout(in);
		nick_destroy(in);
		return (1);
	}
	return (0);
}

static int
cb_sjoin(int argc, char * const argv[])
{
	struct ircyka_channel *ic;
	struct ircyka_nick *in;
	struct ircyka_join *ij;
	char *p, *bp, *cp, nick[IRCYKA_MAX_NICKLEN];
	const char *errstr;
	u_int32_t flags = 0;
	time_t tm;

	if (argc > 5) {
		if ((ic = channel_find(argv[3])) == NULL &&
		    (ic = channel_create(argv[3])) != NULL)
			ic->ic_flags = channel_flags(argv[4], ic->ic_flags);

		if (ic != NULL) {
			tm = strtonum(argv[2], 0, UINT_MAX, &errstr);
			ic->ic_time = (errstr == NULL) ? tm : time(NULL);
			if (ircyka_phase > 1 && ic->ic_count == 0 &&
			    irc_joinall && LIST_EMPTY(&ic->ic_joins))
				(void)irc_join(ic, irc_nick);

			bp = cp = argv[5];
		next:	(void)strlcpy(p = nick, cp, sizeof(nick));
			if ((cp = strchr(p, ' ')) != NULL) {
				*cp++ = '\0';
				cp = (bp += strlen(p) + 1);
			}
			while (*p == '@' || *p == '+' || *p == '%') {
				switch (*p++) {
				case '@':
					flags |= IJF_OPER;
					break;
				case '%':
					flags |= IJF_HALFOP;
					break;
				case '+':
					flags |= IJF_VOICE;
					break;
				}
			}
			if ((in = nick_find(p)) != NULL &&
			    (ij = join_create(ic, in, flags)) != NULL &&
			    ircyka_phase > 1 && (in->in_flags & INF_LOGON) &&
			    (in->in_flags & INF_AUTO) && ic->ic_count != 0 &&
			    acldb_get(ircyka_acldb, ic->ic_channel,
			    in->in_user, &flags) == 0)
				(void)irc_mode(in->in_nick, ic->ic_channel,
				    flags);
			if (cp != NULL)
				goto next;
			return (1);
		} else
			return (0);
	}
	return (0);
}

static int
cb_squit(int argc, char * const argv[])
{
	if (argc > 2) {
		if (charset_strcmp(argv[2], irc_hostname) == 0)
			ircyka_die++;
		return (1);
	}
	return (0);
}

int
match_irc_init(void)
{
	RB_INIT(&ircyka_irc_handlers);

	if (irc_register("KICK", cb_kick) == NULL ||
	    irc_register("KILL", cb_kill) == NULL ||
	    irc_register("MODE", cb_mode) == NULL ||
	    irc_register("NICK", cb_nick) == NULL ||
	    irc_register("PART", cb_part) == NULL ||
	    irc_register("PING", cb_ping) == NULL ||
	    irc_register("PONG", cb_pong) == NULL ||
	    irc_register("PRIVMSG", cb_privmsg) == NULL ||
	    irc_register("QUIT", cb_quit) == NULL ||
	    irc_register("SJOIN", cb_sjoin) == NULL ||
	    irc_register("SQUIT", cb_squit) == NULL)
		return (-1);

	return (0);
}

void
match_irc(char *s)
{
	char *argv[IRCYKA_MAX_ARGS];
	int argc = IRCYKA_MAX_ARGS;
	struct ircyka_handler *ih;
	struct ircyka_callback *icb;

	if (ircyka_debug)
		(void)fprintf(stderr, "< %s\n", s);

	if (irc_parse(&argc, argv, s) == 0 && argc > 1 &&
	    (ih = handler_find(&ircyka_irc_handlers, argv[1])) != NULL) {
		LIST_FOREACH(icb, &ih->ih_callbacks, icb_entry)
			if (icb->icb_cb(argc, argv) != 0)
				break;
	}
}
