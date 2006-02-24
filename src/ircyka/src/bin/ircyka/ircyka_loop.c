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

#include <sys/types.h>
#include <err.h>
#include <string.h>
#include <time.h>

#include "ircyka/ircyka.h"


int ircyka_die;
int ircyka_phase;


void
ircyka_loop(int fd)
{
	char buf[IRCYKA_MAX_BUFLEN];
	struct ircyka_channel *ic;
	struct ircyka_nick *in;
	ssize_t nb = 0, off = 0, len;
	char *p, *cp;

	if (irc_pass != NULL)
		(void)irc_printf("PASS %s :TS", irc_pass);
	else
		(void)irc_printf("PASS :TS");

	(void)irc_printf("CAPAB :QS CHW HOPS");
	(void)irc_printf("SERVER %s 1 :%s", irc_hostname, irc_servname);
	(void)irc_printf("SVINFO 5 3 0 :%u", time(NULL));

	ircyka_die = ircyka_phase = 0;
	while (!ircyka_die) {
		bzero(buf + off, sizeof(buf) - off);
		if ((nb = qio_poll(buf + off, sizeof(buf) - off - 1)) <= 0)
			break;

		nb += off;
		if ((cp = strchr(p = buf, '\n')) == NULL) {
			if (nb ==  sizeof(buf) - 1) {
				if (ircyka_debug)
					warnx("qio_poll: Line too long");
				off = 0;
			} else
				off = nb;
			continue;
		}

		do {
			*cp++ = '\0';
			nb -= (len = strlen(p)) + 1;
			if (len > 0 && p[len - 1] == '\r')
				p[--len] = '\0';
			match_irc(p);
			cp = strchr(p = cp, '\n');
		} while (cp != NULL);

		if (nb != 0)
			bcopy(p, buf, nb);
		off = nb;
	}
	ircyka_phase = 0;

	if (ircyka_debug) {
		if (nb < 0)
			warn("qio_poll");
		else
			warnx("qio_poll: Connection closed by server");
	}

	qio_flush();
	if (ircyka_die && nb == 0 && irc_quitmsg != NULL) {
		irc_printf(":%s QUIT :%s", irc_nick, irc_quitmsg);
		qio_finish(IRCYKA_QUIT_TIMEOUT);
	}

	while ((ic = RB_MIN(ircyka_channel_tree, &ircyka_channels)) != NULL)
		channel_destroy(ic);
	while ((in = RB_MIN(ircyka_nick_tree, &ircyka_nicks)) != NULL)
		nick_destroy(in);

}
