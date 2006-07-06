/*
 * $RuOBSD: ircyka_channel.c,v 1.1.1.1 2006/02/24 17:13:31 form Exp $
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

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ircyka/ircyka.h"


struct ircyka_channel_tree ircyka_channels = RB_INITIALIZER(&ircyka_channels);
struct ircyka_hooks channel_hooks = LIST_HEAD_INITIALIZER(&channel_hooks);

static __inline int
channel_compare(struct ircyka_channel *a, struct ircyka_channel *b)
{
	return (charset_strcmp(a->ic_channel, b->ic_channel));
}

RB_GENERATE(ircyka_channel_tree, ircyka_channel, ic_entry, channel_compare)


struct ircyka_channel *
channel_find(const char *channel)
{
	struct ircyka_channel ic;

	bcopy(&channel, &ic.ic_channel, sizeof(ic.ic_channel));
	return (RB_FIND(ircyka_channel_tree, &ircyka_channels, &ic));
}

struct ircyka_channel *
channel_create(const char *channel)
{
	struct ircyka_channel *ic;

	if ((ic = channel_find(channel)) != NULL) {
		errno = EEXIST;
		return (NULL);
	}

	if ((ic = calloc(1, sizeof(*ic))) != NULL) {
		ic->ic_time = time(NULL);
		LIST_INIT(&ic->ic_joins);
		if ((ic->ic_channel = strdup(channel)) == NULL) {
			int save_errno = errno;

			free(ic);
			ic = NULL;
			errno = save_errno;
		} else {
			(void)RB_INSERT(ircyka_channel_tree,
			    &ircyka_channels, ic);
			hook_exec(&channel_hooks, IRCYKA_HOOK_CREATE, ic);
		}
	}

	return (ic);
}

void
channel_destroy(struct ircyka_channel *ic)
{
	struct ircyka_join *ij;

	if (ircyka_log_target != NULL &&
	    charset_strcmp(ic->ic_channel, ircyka_log_target) == 0)
		ircyka_log_target = NULL;

	while ((ij = LIST_FIRST(&ic->ic_joins)) != NULL) {
		LIST_REMOVE(ij, ij_nentry);
		LIST_REMOVE(ij, ij_centry);
		hook_exec(&join_hooks, IRCYKA_HOOK_CREATE, ij);
		free(ij);
	}
	(void)RB_REMOVE(ircyka_channel_tree, &ircyka_channels, ic);
	hook_exec(&channel_hooks, IRCYKA_HOOK_DESTROY, ic);
	free(ic->ic_channel);
	free(ic);
}

u_int32_t
channel_flags(const char *aflags, u_int32_t flags)
{
	size_t i, len;
	u_int32_t mask;
	int action = 0;

	for (i = 0, len = strlen(aflags); i < len; i++) {
		mask = 0;
		switch (aflags[i]) {
		case '-':
			action = -1;
			break;
		case '+':
			action = 1;
			break;
		case 'i':
			mask |= ICF_I;
			break;
		case 'm':
			mask |= ICF_M;
			break;
		case 'n':
			mask |= ICF_N;
			break;
		case 'p':
			mask |= ICF_P;
			break;
		case 's':
			mask |= ICF_S;
			break;
		case 't':
			mask |= ICF_T;
			break;
		}

		switch (action) {
		case -1:
			flags &= ~mask;
			break;
		case 1:
			flags |= mask;
			break;
		}
	}
	return (flags);
}

char *
channel_aflags(u_int32_t flags)
{
	static char aflags[32];

	(void)snprintf(aflags, sizeof(aflags), "%s%s%s%s%s%s",
	    (flags & ICF_I) ? "i" : "", (flags & ICF_M) ? "m" : "",
	    (flags & ICF_N) ? "n" : "", (flags & ICF_P) ? "p" : "",
	    (flags & ICF_S) ? "s" : "", (flags & ICF_T) ? "t" : "");
	return (aflags);
}

void
channel_init(void)
{
	char *chans[IRCYKA_MAX_CHANNELS], **ap;
	char *cp = irc_channels;
	struct ircyka_channel *ic;

	if (cp != NULL) {
		for (ap = chans; ap < &chans[IRCYKA_MAX_CHANNELS - 1] &&
		    (*ap = strsep(&cp, " ")) != NULL;)
			if (**ap != '\0')
				ap++;
		*ap = NULL;

		for (ap = chans; *ap != NULL &&
		    ap < &chans[IRCYKA_MAX_CHANNELS]; ap++) {
			if (**ap != '#') {
				if (ircyka_debug)
					warnx("%s: Illegal channel name", *ap);
				continue;
			}
			if ((cp = strchr(*ap, '+')) != NULL)
				*cp = '\0';
			if ((ic = channel_create(*ap)) != NULL) {
				if (cp != NULL) {
					*cp = '+';
					ic->ic_flags = channel_flags(cp, 0);
				} else
					ic->ic_flags = ICF_T | ICF_N;
				ic->ic_flags |= ICF_PERSIST;
			}
		}
	}
}

int
channel_valid(const char *channel)
{
	size_t i, len;

	if ((len = strlen(channel)) < 2 || channel[0] != '#')
		return (0);

	for (i = 1; i < len; i++)
		if ((u_char)channel[i] <= ' ' || channel[i] == ',')
			return (0);

	return (1);
}
