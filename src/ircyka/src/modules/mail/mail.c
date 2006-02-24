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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ircyka/ircyka.h"
#include "ircyka/nickdb.h"


#define MAILDIR			"mail"


static void
mail_alarm(void *arg)
{
	struct ircyka_nick *in;
	char mbox[MAXPATHLEN];
	static int active;

	if (active || ircyka_phase < 2)
		return;

	active++;
	RB_FOREACH(in, ircyka_nick_tree, &ircyka_nicks) {
		if (!(in->in_flags & INF_LOGON) || (in->in_flags & INF_MAIL))
			continue;
		(void)snprintf(mbox, sizeof(mbox), "%s/%s/%s", ircyka_conf_dir,
		    MAILDIR, in->in_user);
		if (access(mbox, R_OK) == 0) {
			in->in_flags |= INF_MAIL;
			irc_privmsg(in->in_nick, "You have mail");
		}
	}
	--active;
}

static int
mail_read(const char *nick, const char *target, int my)
{
	char mbox[MAXPATHLEN], user[IRCYKA_MAX_NICKLEN];
	char buf[IRCYKA_MAX_STRLEN];
	int save_errno, error;
	FILE *fp;

	(void)strlcpy(user, nick, sizeof(user));
	charset_strtolower(user);
	(void)snprintf(mbox, sizeof(mbox), "%s/%s/%s", ircyka_conf_dir,
	    MAILDIR, user);

	if ((fp = fopen(mbox, "r")) == NULL) {
		if (errno == ENOENT) {
			irc_privmsg(target, "No mail for %s", user);
			return (0);
		} else
			return (-1);
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		size_t len = strlen(buf);

		while (len > 1 &&
		    (buf[len - 1] == '\r' || buf[len - 1] == '\n'))
			buf[--len] = '\0';
		if (len != 0)
			irc_privmsg(target, "%s", buf);
	}
	if (!(error = ferror(fp)) && my)
		error = unlink(mbox) < 0;

	save_errno = errno;
	(void)fclose(fp);
	errno = save_errno;
	return (error ? -1 : 0);
}

static int
mail_send(const char *fnick, const char *tnick, const char *msg)
{
	char mbox[MAXPATHLEN], user[IRCYKA_MAX_NICKLEN];
	char buf[IRCYKA_MAX_STRLEN], ts[21];
	struct nick n;
	FILE *fp;
	time_t tm;
	int error, save_errno;

	(void)strlcpy(user, tnick, sizeof(user));
	charset_strtolower(user);
	if (nickdb_find(ircyka_nickdb, user, &n) < 0) {
		if (errno == ENOENT)
			errno = ESRCH;
		return (-1);
	}

	(void)snprintf(mbox, sizeof(mbox), "%s/%s/%s", ircyka_conf_dir,
	    MAILDIR, user);

	tm = time(NULL);
	(void)strftime(ts, sizeof(ts), "%d-%b-%Y %H:%M:%S", localtime(&tm));
	(void)snprintf(buf, sizeof(buf), "%s <%s> %s", ts, fnick, msg);

	if ((fp = fopen(mbox, "a")) == NULL)
		return (-1);
	(void)fprintf(fp, "%s\n", buf);

	error = ferror(fp);
	save_errno = errno;
	(void)fclose(fp);
	errno = save_errno;
	return (error ? -1 : 0);
}

static int
cb_mail(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	char *args[3], *p;
	int nargs = 3, save_errno, my;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "MAIL -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "MAIL -- Not logged in");
		return (1);
	}

	if (argv[3] == NULL) {
		if (mail_read(in->in_user, argv[4], 1) < 0)
			goto oserr;
		in->in_flags &= ~INF_MAIL;
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (nargs < 2) {
			my = charset_strcmp(args[0], in->in_nick) == 0;
			if (!my && !(in->in_flags & INF_PRIV)) {
				free(p);
				errno = EPERM;
				goto oserr;
			}
			if (mail_read(args[0], argv[4], my) < 0) {
			error:	save_errno = errno;
				free(p);
				errno = save_errno;
				goto oserr;
			}
			if (my)
				in->in_flags &= ~INF_MAIL;
		} else {
			if (mail_send(in->in_user, args[0], args[1]) < 0)
				goto error;
		}
		free(p);
	} else {
	oserr:	if (errno == ESRCH)
			irc_privmsg(argv[4], "MAIL -- No such account");
		else
			irc_privmsg(argv[4], "MAIL -- %s", strerror(errno));
	}

	return (1);
}


static int
ircyka_module_entry(int func, const char *target, const char *arg)
{
	static struct ircyka_callback *icb;
	static struct ircyka_alarm *ia;

	switch (func) {
	case IME_LOAD:
		if ((icb = cmd_register("MAIL", cb_mail)) == NULL)
			return (-1);
		if ((ia = alarm_add(mail_alarm, NULL)) == NULL) {
			int save_errno = errno;

			cmd_unregister(icb);
			errno = save_errno;
			return (-1);
		}
		break;
	case IME_UNLOAD:
		alarm_del(ia);
		cmd_unregister(icb);
		break;
	}
	return (0);
}

IRCYKA_MODULE("mail", "IRCyka mailer");
