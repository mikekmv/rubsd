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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ircyka/ircyka.h"
#include "ircyka/acldb.h"


int
irc_printf(const char *fmt, ...)
{
	char buf[IRCYKA_MAX_STRLEN];
	size_t len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (len >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	return (qio_printf("%s\r\n", buf));
}

int
irc_privmsg(const char *target, const char *fmt, ...)
{
	char buf[IRCYKA_MAX_STRLEN];
	size_t len;
	va_list ap;

	if (ircyka_phase < 2 || target == NULL) {
		errno = ircyka_phase < 2 ? EWOULDBLOCK : EINVAL;
		return (-1);
	}

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (len >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	return (irc_printf(":%s PRIVMSG %s :%s", irc_nick, target, buf));
}

int
irc_notice(const char *target, const char *fmt, ...)
{
	char buf[IRCYKA_MAX_STRLEN];
	size_t len;
	va_list ap;

	if (ircyka_phase < 2 || target == NULL) {
		errno = ircyka_phase < 2 ? EWOULDBLOCK : EINVAL;
		return (-1);
	}

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (len >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	return (irc_printf(":%s NOTICE %s :%s", irc_nick, target, buf));
}

int
irc_mode(const char *nick, const char *channel, u_int32_t flags)
{
	char aflags[16];
	int rc = 0;

	if (ircyka_phase < 2) {
		errno = EWOULDBLOCK;
		return (-1);
	}

	if ((flags & (AF_HALFOP | AF_OPER)) == (AF_HALFOP | AF_OPER))
		flags &= ~AF_HALFOP;
	(void)snprintf(aflags, sizeof(aflags), "%s%s%s",
	    (flags & AF_HALFOP) ? "h" : "", (flags & AF_OPER) ? "o" : "",
	    (flags & AF_VOICE) ? "v" : "");
	switch (strlen(aflags)) {
	case 1:
		rc = irc_printf(":%s MODE %s +%s %s", irc_nick, channel,
		    aflags, nick);
		break;
	case 2:
		rc = irc_printf(":%s MODE %s +%s %s %s", irc_nick, channel,
		    aflags, nick, nick);
		break;
	}

	return (rc);
}

int
irc_invite(struct ircyka_channel *ic, const char *nick)
{
	if (ircyka_phase < 2) {
		errno = EWOULDBLOCK;
		return (-1);
	}
	return (irc_printf(":%s INVITE %s %s %u", irc_nick, nick,
	    ic->ic_channel, ic->ic_time));
}

int
irc_join(struct ircyka_channel *ic, const char *nick)
{
	int rc;

	if (ircyka_phase < 1) {
		errno = EWOULDBLOCK;
		return (-1);
	}
	if ((rc = irc_printf(":%s SJOIN %u %s +%s :@%s", irc_hostname,
	    ic->ic_time, ic->ic_channel, channel_aflags(ic->ic_flags),
	    nick)) == 0) {
		ic->ic_count++;
		if (log_target != NULL && ircyka_log_target == NULL &&
		    charset_strcmp(ic->ic_channel, log_target) == 0)
			ircyka_log_target = ic->ic_channel;
	}
	return (rc);
}

int
irc_part(struct ircyka_channel *ic, const char *nick, const char *reason)
{
	int rc;

	if (ircyka_phase < 2) {
		errno = EWOULDBLOCK;
		return (-1);
	}

	if (ic->ic_count == 0) {
		errno = ENOENT;
		return (-1);
	}

	if (reason != NULL)
		rc = irc_printf(":%s PART %s :%s", nick, ic->ic_channel,
		    reason);
	else
		rc = irc_printf(":%s PART %s", nick, ic->ic_channel);
	if (rc == 0 && --ic->ic_count == 0 && LIST_EMPTY(&ic->ic_joins) &&
	    !(ic->ic_flags & ICF_PERSIST))
		channel_destroy(ic);
	return (rc);
}

int
irc_parse(int *argc, char **argv, char *str)
{
	char *cp;
	int i;

	if (*argc < 2) {
		errno = EINVAL;
		return (-1);
	}

	if (str[0] == ':') {
		argv[0] = ++str;
		if ((cp = strchr(str, ' ')) != NULL) {
			*cp++ = '\0';
			str = cp;
		} else {
			*argc = 1;
			return (0);
		}
	} else
		argv[0] = NULL;

	for (i = 1; i < *argc; i++) {
		if (*str == ':') {
			argv[i] = ++str;
			break;
		}
		argv[i] = str;
		if ((cp = strchr(str, ' ')) == NULL)
			break;
		*cp++ = '\0';
		str = cp; 
	}

	if (i == *argc)
		return (1);
	else
		*argc = ++i;

	return (0);
}
