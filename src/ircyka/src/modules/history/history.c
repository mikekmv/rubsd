/*
 * $RuOBSD: history.c,v 1.1 2006/07/07 16:22:14 form Exp $
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


#define MAX_HIST_MESSAGES		20


struct hist_message {
	char				*hm_message;
	time_t				hm_time;
	TAILQ_ENTRY(hist_message)	hm_entry;
};

struct hist_channel {
	const struct ircyka_channel	*hc_ic;
	int				hc_msgcount;
	TAILQ_HEAD(, hist_message)	hc_messages;
	RB_ENTRY(hist_channel)		hc_entry;
};


static RB_HEAD(hist_channel_tree, hist_channel) hist_channels =
    RB_INITIALIZER(&hist_channels);


static __inline int
channel_compare(struct hist_channel *a, struct hist_channel *b)
{
	if (a->hc_ic < b->hc_ic)
		return (-1);
	if (a->hc_ic > b->hc_ic)
		return (1);
	return (0);
}

RB_PROTOTYPE(hist_channel_tree, hist_channel, hc_entry, channel_compare)
RB_GENERATE(hist_channel_tree, hist_channel, hc_entry, channel_compare)


static void
messages_cleanup(struct hist_channel *hc)
{
	struct hist_message *hm;

	while ((hm = TAILQ_FIRST(&hc->hc_messages)) != NULL) {
		TAILQ_REMOVE(&hc->hc_messages, hm, hm_entry);
		free(hm->hm_message);
		free(hm);
	}
}

static void
history_cleanup(void)
{
	struct hist_channel *hc;

	while ((hc = RB_MIN(hist_channel_tree, &hist_channels)) != NULL) {
		RB_REMOVE(hist_channel_tree, &hist_channels, hc);
		messages_cleanup(hc);
		free(hc);
	}
}

static int
cb_hist(int argc, char * const argv[])
{
	struct hist_channel *hc, hch;
	struct hist_message *hm;
	struct ircyka_nick *in;
	struct ircyka_join *ij;
	struct tm tm, *mtm;
	char ts[21];
	time_t t;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "HIST -- Who are you?");
		return (1);
	}

	if (argv[1][0] != '#') {
		if (argv[3] == NULL) {
			irc_privmsg(argv[4], "HIST -- Channel name required");
			return (1);
		}
		hch.hc_ic = channel_find(argv[3]);
	} else
		hch.hc_ic = channel_find(argv[3] != NULL ? argv[3] : argv[1]);

	if (hch.hc_ic == NULL) {
		irc_privmsg(argv[4], "HIST -- Channel not found");
		return (1);
	}

	LIST_FOREACH(ij, &in->in_joins, ij_nentry)
		if (ij->ij_ic == hch.hc_ic)
			break;

	if (ij == NULL && !(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "HIST -- You're not joined to %s",
		    hch.hc_ic->ic_channel);
		return (1);
	}

	if ((hc = RB_FIND(hist_channel_tree, &hist_channels, &hch)) == NULL)
		return (1);

	t = time(NULL);
	(void)localtime_r(&t, &tm);
	TAILQ_FOREACH(hm, &hc->hc_messages, hm_entry) {
		mtm = localtime(&hm->hm_time);
		if (tm.tm_year != mtm->tm_year)
			(void)strftime(ts, sizeof(ts), "%d-%b-%Y %H:%M:%S",
			    mtm);
		else if (tm.tm_mday != mtm->tm_mday || tm.tm_mon != mtm->tm_mon)
			(void)strftime(ts, sizeof(ts), "%d-%b %H:%M:%S", mtm);
		else
			(void)strftime(ts, sizeof(ts), "%H:%M:%S", mtm);
		(void)irc_privmsg(argv[4], "%s %s", ts, hm->hm_message);
	}

	return (1);
}

static int
cb_privmsg(int argc, char * const argv[])
{
	struct hist_channel *hc, hch;
	struct hist_message *hm;
	char *message;
	int rc;

	if (argv[0] == NULL || argc != 4 || argv[2][0] != '#' ||
	    argv[3][0] == '!')
		return (0);

	if ((hch.hc_ic = channel_find(argv[2])) == NULL ||
	    (hc = RB_FIND(hist_channel_tree, &hist_channels, &hch)) == NULL)
		return (0);

	if (argv[3][0] == '\001') {
		size_t len;

		len = strlen(argv[3]);
		if (strncmp(argv[3], "\001ACTION ", 8) == 0)
			rc = asprintf(&message, "* %s %.*s",
			    argv[0], (int)len - 9, &argv[3][8]);
		else
			return (0);
	} else
		rc = asprintf(&message, "<%s> %s", argv[0], argv[3]);

	if (rc < 0)
		return (0);

	if (hc->hc_msgcount == MAX_HIST_MESSAGES) {
		hm = TAILQ_FIRST(&hc->hc_messages);
		TAILQ_REMOVE(&hc->hc_messages, hm, hm_entry);
		free(hm->hm_message);
		--hc->hc_msgcount;
	} else if ((hm = malloc(sizeof(*hm))) == NULL) {
		free(message);
		return (0);
	}

	hm->hm_time = time(NULL);
	hm->hm_message = message;
	hc->hc_msgcount++;
	TAILQ_INSERT_TAIL(&hc->hc_messages, hm, hm_entry);

	return (0);
}

static void
ih_channel(int type, const void *data)
{
	struct hist_channel *hc, hch;

	switch (type) {
	case IRCYKA_HOOK_CREATE:
		if ((hc = malloc(sizeof(*hc))) != NULL) {
			hc->hc_ic = data;
			hc->hc_msgcount = 0;
			TAILQ_INIT(&hc->hc_messages);
			(void)RB_INSERT(hist_channel_tree, &hist_channels, hc);
		}
		break;
	case IRCYKA_HOOK_DESTROY:
		hch.hc_ic = data;
		if ((hc = RB_FIND(hist_channel_tree, &hist_channels,
		    &hch)) != NULL) {
			RB_REMOVE(hist_channel_tree, &hist_channels, hc);
			messages_cleanup(hc);
			free(hc);
		}
		break;
	}
}

static int
ircyka_module_entry(int func, const char *target, const char *arg)
{
	static struct ircyka_callback *cicb, *iicb;
	static struct ircyka_hook *ih;
	struct ircyka_channel *ic;
	struct hist_channel *hc;
	int save_errno;

	switch (func) {
	case IME_LOAD:
		if ((cicb = cmd_register("HIST", cb_hist)) == NULL ||
		    (iicb = irc_register("PRIVMSG", cb_privmsg)) == NULL ||
		    (ih = channel_hook(ih_channel)) == NULL) {
			save_errno = errno;
			if (cicb != NULL)
				cmd_unregister(cicb);
			if (iicb != NULL)
				irc_unregister(iicb);
			errno = save_errno;
			return (-1);
		}

		RB_FOREACH(ic, ircyka_channel_tree, &ircyka_channels) {
			if ((hc = malloc(sizeof(*hc))) == NULL) {
				save_errno = errno;
				history_cleanup();
				errno = save_errno;
				return (-1);
			}
			hc->hc_ic = ic;
			hc->hc_ic = ic;
			hc->hc_msgcount = 0;
			TAILQ_INIT(&hc->hc_messages);
			(void)RB_INSERT(hist_channel_tree, &hist_channels, hc);
		}
		break;
	case IME_UNLOAD:
		channel_unhook(ih);
		irc_unregister(iicb);
		irc_unregister(cicb);
		history_cleanup();
		break;
	}
	return (0);
}

IRCYKA_MODULE("history", "Channel messages history");
