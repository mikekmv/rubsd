/*
 * $RuOBSD: ircyka_nick.c,v 1.1.1.1 2006/02/24 17:13:31 form Exp $
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ircyka/ircyka.h"


struct ircyka_nick_tree ircyka_nicks = RB_INITIALIZER(&ircyka_nicks);
struct ircyka_hooks nick_hooks = LIST_HEAD_INITIALIZER(&nick_hooks);


static __inline int
nick_compare(struct ircyka_nick *a, struct ircyka_nick *b)
{
	return (charset_strcmp(a->in_nick, b->in_nick));
}

RB_GENERATE(ircyka_nick_tree, ircyka_nick, in_entry, nick_compare)


struct ircyka_nick *
nick_find(const char *nick)
{
	struct ircyka_nick in;

	(void)strlcpy(in.in_nick, nick, sizeof(in.in_nick));
	return (RB_FIND(ircyka_nick_tree, &ircyka_nicks, &in));
}

struct ircyka_nick *
nick_create(const char *nick, const char *host)
{
	struct ircyka_nick *in;

	if ((in = nick_find(nick)) != NULL) {
		errno = EEXIST;
		return (NULL);
	}

	if ((in = calloc(1, sizeof(*in))) != NULL) {
		in->in_time = time(NULL);
		(void)strlcpy(in->in_nick, nick, sizeof(in->in_nick));
		if ((in->in_host = strdup(host)) == NULL) {
			int save_errno = errno;

			free(in);
			in = NULL;
			errno = save_errno;
		} else {
			LIST_INIT(&in->in_joins);
			(void)RB_INSERT(ircyka_nick_tree, &ircyka_nicks, in);
			hook_exec(&nick_hooks, IRCYKA_HOOK_CREATE, in);
		}

	}
	return (in);
}

int
nick_change(struct ircyka_nick *in, const char *nick)
{
	(void)RB_REMOVE(ircyka_nick_tree, &ircyka_nicks, in);
	bzero(in->in_nick, sizeof(in->in_nick));
	(void)strlcpy(in->in_nick, nick, sizeof(in->in_nick));
	if (RB_INSERT(ircyka_nick_tree, &ircyka_nicks, in) != NULL) {
		if (in->in_host != NULL)
			free(in->in_host);
		free(in);
		errno = EEXIST;
		return (-1);
	}
	hook_exec(&nick_hooks, IRCYKA_HOOK_CHANGE, in);
	return (0);
}

void
nick_destroy(struct ircyka_nick *in)
{
	struct ircyka_join *ij;

	if (ircyka_log_target != NULL &&
	    charset_strcmp(in->in_nick, ircyka_log_target) == 0)
		ircyka_log_target = NULL;

	while ((ij = LIST_FIRST(&in->in_joins)) != NULL)
		join_destroy(ij);
	(void)RB_REMOVE(ircyka_nick_tree, &ircyka_nicks, in);
	hook_exec(&nick_hooks, IRCYKA_HOOK_DESTROY, in);
	if (in->in_host != NULL)
		free(in->in_host);
	free(in);
}

void
nick_login(struct ircyka_nick *in, const char *user, int priv)
{
	char ts[21];
	time_t tm;

	in->in_flags |= INF_LOGON;
	if (priv)
		in->in_flags |= INF_PRIV;
	(void)strlcpy(in->in_user, user, sizeof(in->in_user));
	hook_exec(&nick_hooks, IRCYKA_HOOK_CHANGE, in);
	if (ircyka_log_target != NULL) {
		tm = time(NULL);
		(void)strftime(ts, sizeof(ts), "%d-%b-%Y %H:%M:%S",
		    localtime(&tm));
		irc_privmsg(ircyka_log_target,
		    "\002\00303%s LOGIN NICK %s AS %s FROM %s", ts,
		    in->in_nick, in->in_user, in->in_host);
	}
}

void
nick_logout(struct ircyka_nick *in)
{
	char ts[21];
	time_t tm;

	if (ircyka_log_target != NULL) {
		tm = time(NULL);
		(void)strftime(ts, sizeof(ts), "%d-%b-%Y %H:%M:%S",
		    localtime(&tm));
		irc_privmsg(ircyka_log_target,
		    "\002\00303%s LOGOUT NICK %s (%s) FROM %s", ts,
		    in->in_nick, in->in_user, in->in_host);
	}
	bzero(in->in_user, sizeof(in->in_user));
	in->in_flags = 0;
	hook_exec(&nick_hooks, IRCYKA_HOOK_CHANGE, in);
}

char *
nick_aflags(u_int32_t flags)
{
	static char aflags[16];

	(void)snprintf(aflags, sizeof(aflags), "%s%s%s%s%s",
	    flags & INF_AUTO ? "A" : "", flags & INF_CHANOUT ? "C" : "",
	    flags & INF_LOGON ? "L" : "", flags & INF_MAIL ? "M" : "",
	    flags & INF_PRIV ? "P" : "");
	return (aflags);
}

int
nick_flags(const char *aflags, u_int32_t *flags)
{
	u_int32_t mask, nflags = *flags;
	size_t len, i;
	int op = 0;

	for (i = 0, len = strlen(aflags); i < len; i++) {
		mask = 0;
		switch (aflags[i]) {
		case '-':
			op = -1;
			continue;
		case '+':
			op = 1;
			continue;
		case 'a':
		case 'A':
			mask |= INF_AUTO;
			break;
		case 'c':
		case 'C':
			mask |= INF_CHANOUT;
			break;
		case 'l':
		case 'L':
			mask |= INF_LOGON;
			break;
		case 'm':
		case 'M':
			mask |= INF_MAIL;
			break;
		case 'p':
		case 'P':
			mask |= INF_PRIV;
			break;
		default:
			return ((int)aflags[i]);
		}

		if (op == -1)
			nflags &= ~mask;
		else if (op == 1)
			nflags |= mask;
		else
			return (-1);
	}

	*flags = nflags;
	return (0);
}
