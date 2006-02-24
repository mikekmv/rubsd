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

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ircyka/ircyka.h"
#include "ircyka/nickdb.h"
#include "ircyka/acldb.h"

#include <stdio.h>

struct ircyka_handler_tree ircyka_cmd_handlers;

static int
cb_abo(int argc, char * const argv[])
{
	struct ircyka_nick *in;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "ABO -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "ABO -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "ABO -- Privileged command");
		return (1);
	}

	qio_flush();
	return (1);
}

static int
cb_acl(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct acl *a;
	char *args[4], *p;
	int nargs = 4, rc, ch, i;
	u_int32_t flags = 0;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "ACL -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "ACL -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "ACL -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (args[0][0] != '#') {
			free(p);
			goto error;
		}
		for (i = 0; i < nargs; i++)
			charset_strtolower(args[i]);
		if (nargs == 1) {
			if ((rc = acldb_find(ircyka_acldb, args[0],
			    NULL, &a)) >= 0) {
				irc_privmsg(argv[4], "Nick             Flags");
				irc_privmsg(argv[4], "---------------- -----");
				for (i = 0; i < rc; i++)
					irc_privmsg(argv[4], "%-16s %s",
					     a[i].a_nick,
					     acldb_aflags(a[i].a_flags));
				free(a);
			} else if (errno == ENOENT) {
				irc_privmsg(argv[4], "ACL -- No acls for %s",
				    args[0]);
				return (1);
			}
		} else {
			if ((rc = acldb_get(ircyka_acldb, args[0], args[1],
			    &flags)) == 0 || (nargs > 2 && errno == ENOENT)) {
				if (nargs == 2) {
					irc_privmsg(argv[4],
					    "Nick             Flags");
					irc_privmsg(argv[4],
					    "---------------- -----");
					irc_privmsg(argv[4], "%-16s %s",
					    args[1], acldb_aflags(flags));
					free(p);
					return (1);
				}
				if ((ch = acldb_flags(args[2], &flags)) != 0) {
					free(p);
					if (ch < 0)
						goto error;
					irc_privmsg(argv[4],
					    "ACL -- Unsupported flag - %c", ch);
					return (1);
				}
				if (rc == 0) {
					if (flags == 0)
						rc = acldb_del(ircyka_acldb,
						    args[0], args[1]);
					else
						rc = acldb_update(ircyka_acldb,
						    args[0], args[1], flags);
				} else
					rc = acldb_add(ircyka_acldb,
					    args[0], args[1], flags);
				if (rc == 0)
					rc = acldb_sync(ircyka_acldb);
			}
		}
		if (rc < 0) {
			if (errno == ENOENT)
				irc_privmsg(argv[4], "ACL -- No such acl");
			else {
				int save_errno = errno;

				free(p);
				errno = save_errno;
				goto oserr;
			}
		}
		free(p);
	} else {
		if (errno == 0)
		error:	irc_privmsg(argv[4], "ACL -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "ACL -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_add(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct nick n;
	char *args[4], *p, *aflags = NULL, *passwd = NULL;
	int nargs = 4, ch;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "ADD -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "ADD -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "ADD -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (nargs < 2) {
			free(p);
			goto error;
		}

		charset_strtolower(args[0]);
		if (args[1][0] == '+' || args[1][0] == '-') {
			charset_strtolower(args[1]);
			aflags = args[1];
			passwd = args[2];
		} else {
			if (nargs > 2) {
				free(p);
				nargs = 3;
				if ((p = match_cmd_parse(&nargs, args,
				    argv[3])) == NULL)
					goto oserr;
			}
			passwd = args[1];
		}


		bzero(&n, sizeof(n));
		(void)strlcpy(n.n_nick, args[0], sizeof(n.n_nick));

		if (aflags != NULL &&
		    (ch = nickdb_flags(aflags, &n.n_flags)) != 0) {
			free(p);
			irc_privmsg(argv[4],
			    "ADD -- Unsupported flag - %c", ch);
			return (1);
		}
		if (passwd != NULL)
			nickdb_passwd(passwd, n.n_passwd);

		free(p);
		if (nickdb_add(ircyka_nickdb, &n) < 0) {
			if (errno != EEXIST)
				goto oserr;
			irc_privmsg(argv[4], "ADD -- Account already exists");
		} else if (nickdb_sync(ircyka_nickdb) < 0)
			goto error;
	} else {
		if (errno == 0)
		error:	irc_privmsg(argv[4], "ADD -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "ADD -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_bro(int argc, char * const argv[])
{
	struct ircyka_channel *ic;
	struct ircyka_nick *in;
	char *args[3], *p;
	int nargs = 3, aflag = 0, lflag = 0, nflag = 0;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "BRO -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "BRO -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[1], "BRO -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (nargs != 2) {
			free(p);
			goto error;
		}

		if (strcasecmp(args[0], "-a") == 0)
			aflag++;
		else if (strcasecmp(args[0], "-l") == 0)
			lflag++;
		else if (strcasecmp(args[0], "-n") == 0)
			nflag++;

		if (aflag) {
			RB_FOREACH(ic, ircyka_channel_tree, &ircyka_channels) {
				if (ic->ic_count == 0)
					continue;
				irc_privmsg(ic->ic_channel, "%s", args[1]);
			}
		} else if (lflag || nflag) {
			RB_FOREACH(in, ircyka_nick_tree, &ircyka_nicks) {
				if (lflag && !(in->in_flags & INF_LOGON))
					continue;
				irc_privmsg(in->in_nick, "%s", args[1]);
			}
		} else {
			const char *target = NULL;

			if (args[0][0] == '#') {
				if ((ic = channel_find(args[0])) != NULL);
					target = ic->ic_channel;
			} else if ((in = nick_find(args[0])) != NULL)
				target = in->in_nick;
			if (target != NULL)
				irc_privmsg(target, "%s", args[1]);
			else
				irc_privmsg(argv[4],
				    "BRO -- Target not found");
		}
		free(p);
	} else {
		if (errno == 0)
		error:	irc_privmsg(argv[4], "BRO -- Syntax error");
		else
			irc_privmsg(argv[4], "BRO -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_bye(int argc, char * const argv[])
{
	struct ircyka_nick *in;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "BYE -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "BYE -- Not logged in");
		return (1);
	}

	nick_logout(in);
	return (1);
}

static int
cb_del(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	char nick[IRCYKA_MAX_NICKLEN];
	int rc;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "DEL -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "DEL -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "DEL -- Privileged command");
		return (1);
	}

	if (argv[3] == NULL) {
		irc_privmsg(argv[4], "DEL -- Syntax error");
		return (1);
	}

	(void)strlcpy(nick, argv[3], sizeof(nick));
	charset_strtolower(nick);

	if ((rc = nickdb_del(ircyka_nickdb, nick)) == 0)
		rc = nickdb_sync(ircyka_nickdb);

	if (rc < 0) {
		if (errno == ENOENT)
			irc_privmsg(argv[4], "DEL -- Account does not exist");
		else
			irc_privmsg(argv[4], "DEL -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_die(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	char *p;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "DIE -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "DIE -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "DIE -- Privileged command");
		return (1);
	}

	if (argv[3] != NULL && (p = strdup(argv[3])) != NULL)
		irc_quitmsg = p;
	ircyka_die++;

	return (1);
}

static int
cb_exe(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_module *im;
	char *args[3], *p;
	int nargs = 3;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "EXE -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "EXE -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "EXE -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if ((im = module_find(args[0])) == NULL) {
			free(p);
			irc_privmsg(argv[4], "EXE -- Module not loaded");
			return (1);
		}

		if (im->im_exec(IME_EXEC, argv[4], args[1]) < 0) {
			int save_errno = errno;

			free(p);
			errno = save_errno;
			goto oserr;
		}
		free(p);
	} else {
		if (errno == 0)
			irc_privmsg(argv[4], "EXE -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "EXE -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_hel(int argc, char * const argv[])
{
	struct nick n;
	struct ircyka_nick *in;
	struct ircyka_join *ij;
	char *p, *args[3], nick[IRCYKA_MAX_NICKLEN];
	int nargs = 3;
	u_int32_t flags;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "HEL -- Who are you?");
		return (1);
	}

	if (in->in_flags & INF_LOGON) {
		irc_privmsg(argv[4], "HEL -- Other user logged on");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		(void)strlcpy(nick, nargs > 1 ? args[0] : argv[0],
		    sizeof(nick));
		charset_strtolower(nick);

		if (nickdb_auth(ircyka_nickdb, nick,
		    args[nargs - 1], &n) < 0) {
			if (errno == EACCES || errno == ENOENT)
				irc_privmsg(argv[4], "HEL -- Invalid account");
			else
				irc_privmsg(argv[4], "HEL -- %s",
				    strerror(errno));
		} else if (n.n_flags & NF_DISABLED)
			irc_privmsg(argv[4], "HEL -- Account disabled");
		else {
			nick_login(in, n.n_nick, n.n_flags & NF_PRIV);
			if (n.n_flags & NF_CHANOUT)
				in->in_flags |= INF_CHANOUT;
			if (n.n_flags & NF_AUTO) {
				in->in_flags |= INF_AUTO;
				LIST_FOREACH(ij, &in->in_joins, ij_nentry) {
					if (acldb_get(ircyka_acldb,
					    ij->ij_ic->ic_channel, in->in_user,
					    &flags) == 0)
						(void)irc_mode(in->in_nick,
						    ij->ij_ic->ic_channel,
						    flags);
				}
			}
		}
		free(p);
	} else {
		if (errno == 0)
			irc_privmsg(argv[4], "HEL -- Syntax error");
		else
			irc_privmsg(argv[4], "HEL -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_inv(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_channel *ic;
	struct ircyka_join *ij;
	char *channel;
	u_int32_t flags;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "INV -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "INV -- Not logged in");
		return (1);
	}

	if (argv[3] != NULL) {
		if ((ic = channel_find(argv[3])) == NULL) {
			irc_privmsg(argv[4], "INV -- Not such channel");
			return (1);
		}
		LIST_FOREACH(ij, &in->in_joins, ij_nentry)
			if (ij->ij_ic == ic)
				break;
		if (ij != NULL) {
			irc_privmsg(argv[4],
			    "INV -- You're already joined %s", ic->ic_channel);
			return (1);
		}
		if (!(in->in_flags & INF_PRIV)) {
			if ((channel = strdup(argv[3])) == NULL)
				goto oserr;
			charset_strtolower(channel);
			if (acldb_get(ircyka_acldb, channel, in->in_user,
			    &flags) < 0) {
				int save_errno = errno;

				free(channel);
				errno = save_errno;
				if (errno == ENOENT)
				error:	irc_privmsg(argv[4],
					    "INV -- Privilege violation");
				else
				oserr:	irc_privmsg(argv[4], "INV -- %s",
					    strerror(errno));
				return (1);
			}
			if (!(flags & AF_INVITE))
				goto error;
		}
		if (ic->ic_count != 0)
			(void)irc_invite(ic, in->in_nick);
		else
			irc_privmsg(argv[4], "INV -- Not joined %s",
			    ic->ic_channel);
	} else
		irc_privmsg(argv[4], "INV -- Syntax error");

	return (1);
}

static int
cb_joi(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_channel *ic;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "JOI -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "JOI -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "JOI -- Privileged command");
		return (1);
	}

	if (argv[3] == NULL || argv[3][0] != '#') {
		irc_privmsg(argv[4], "JOI -- Syntax error");
		return (1);
	}

	if ((ic = channel_find(argv[3])) != NULL) {
		if (ic->ic_count != 0) {
			irc_privmsg(argv[4], "JOI -- Already joined %s",
			    ic->ic_channel);
			return (1);
		}
		if (irc_join(ic, irc_nick) < 0)
			goto oserr;
	} else {
		if (!channel_valid(argv[3])) {
			irc_privmsg(argv[4], "JOI -- Invalid channel name");
			return(1);
		}
		if ((ic = channel_create(argv[3])) == NULL) {
		oserr:	irc_privmsg(argv[4], "JOI -- %s", strerror(errno));
			return (1);
		}
		ic->ic_flags |= ICF_N | ICF_T;
		if (irc_join(ic, irc_nick) < 0) {
			int save_errno = errno;

			free(ic);
			errno = save_errno;
			goto oserr;
		}
	}
	ic->ic_flags |= ICF_PERSIST;

	return (1);
}

static int
cb_loa(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	char *args[3], *p;
	int nargs = 3;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "LOA -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "LOA -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "LOA -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (module_load(args[0], argv[4], args[1]) == NULL) {
			int save_errno = errno;

			free(p);
			errno = save_errno;
			if (errno == EEXIST) {
				irc_privmsg(argv[4],
				    "LOA -- Module already loaded");
				return (-1);
			} else if (errno != 0)
				goto oserr;
			irc_privmsg(argv[4], "LOA -- %s", dlerror());
			return (1);;
		}
		free(p);
	} else {
		if (errno == 0)
			irc_privmsg(argv[4], "LOA -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "LOA -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_log(int argc, char * const argv[])
{
	char *target = NULL;
	struct ircyka_channel *ic;
	struct ircyka_nick *in;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "LOG -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "LOG -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "LOG -- Privileged command");
		return (1);
	}

	if (argv[3] == NULL) {
		if (ircyka_log_target == NULL)
			irc_privmsg(argv[4], "Logging is off");
		else
			irc_privmsg(argv[4], "Logging to %s",
			    ircyka_log_target);
		return (1);
	}

	if (strcmp(argv[3], "-") == 0) {
		ircyka_log_target = NULL;
		if (log_target != NULL) {
			free(log_target);
			log_target = NULL;
		}
		return (1);
	}

	if (argv[3][0] == '#') {
		if ((ic = channel_find(argv[3])) != NULL)
			target = ic->ic_channel;
	} else if ((in = nick_find(argv[3])) != NULL)
		target = in->in_nick;

	if (target == NULL) {
		irc_privmsg(argv[4], "LOG -- Target not found");
		return (1);
	}

	if (log_target != NULL) {
		free(log_target);
		log_target = NULL;
	}

	ircyka_log_target = target;
	return (1);
}

static int
cb_lst(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct nick n;
	char nick[IRCYKA_MAX_NICKLEN];
	int rc;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "LST -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "LST -- Not logged in");
		return (1);
	}

	if (argv[3] == NULL)
		rc = nickdb_find(ircyka_nickdb, N_FIRST, &n);
	else {
		(void)strlcpy(nick, argv[3], sizeof(nick));
		charset_strtolower(nick);
		rc = nickdb_find(ircyka_nickdb, nick, &n);
	}

	if (rc == 0) {
		irc_privmsg(argv[4], "Account          Flags Name");
		irc_privmsg(argv[4], "---------------- ----- ----------------");
	}

	if (argv[3] == NULL) {
		while (rc == 0) {
			irc_privmsg(argv[4], "%-16s %-5s %s", n.n_nick,
			    nickdb_aflags(n.n_flags), n.n_name);
			rc = nickdb_find(ircyka_nickdb, N_NEXT, &n);
		}
		if (errno == ENOENT)
			rc = 0;
	} else if (rc == 0)
		irc_privmsg(argv[4], "%-16s %-5s %s", n.n_nick,
		    nickdb_aflags(n.n_flags), n.n_name);

	if (rc < 0) {
		if (errno == ENOENT)
			irc_privmsg(argv[4], "LST -- Account does not exist");
		else
			irc_privmsg(argv[4], "LST -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_mls(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_module *im;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "MLS -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "MLS -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "MLS -- Privileged command");
		return (1);
	}

	if (LIST_EMPTY(&ircyka_modules)) {
		irc_privmsg(argv[4], "No modules loaded");
		return (1);
	}

	irc_privmsg(argv[4], "Module           Rev Flags    Description");
	irc_privmsg(argv[4], "---------------- --- -------- ----------------");

	LIST_FOREACH(im, &ircyka_modules, im_entry)
		irc_privmsg(argv[4], "%-16s %-3d %08x %s", im->im_name,
		    im->im_rev, im->im_flags, im->im_descr);

	return (1);
}

static int
cb_mod(int argc, char * const argv[])
{
	char *args[4], *p, *aflags = NULL, *passwd = NULL;
	struct ircyka_nick *in;
	struct nick n;
	int nargs = 4, rc, ch, save_errno;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "MOD -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "MOD -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "MOD -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (nargs < 2) {
			free(p);
			goto error;
		}

		charset_strtolower(args[0]);
		if (args[1][0] == '+' || args[1][0] == '-') {
			aflags = args[1];
			passwd = args[2];
			charset_strtolower(aflags);
		} else {
			if (nargs > 2) {
				free(p);
				nargs = 3;
				if ((p = match_cmd_parse(&nargs, args,
				    argv[3])) == NULL)
					goto oserr;
			}
			passwd = args[1];
		}
		if ((rc = nickdb_find(ircyka_nickdb, args[0], &n)) == 0) {
			if (aflags != NULL &&
			    (ch = nickdb_flags(aflags, &n.n_flags)) != 0) {
				free(p);
				irc_privmsg(argv[4],
				    "MOD -- Unsupported flag - %c", ch);
				return (1);
			}
			if (passwd != NULL)
				nickdb_passwd(passwd, n.n_passwd);
			if ((rc = nickdb_update(ircyka_nickdb, &n)) == 0)
				rc = nickdb_sync(ircyka_nickdb);
		}
		save_errno = errno;
		free(p);
		errno = save_errno;

		if (rc < 0) {
			if ((errno = save_errno) == ENOENT)
				irc_privmsg(argv[4],
				    "MOD -- Account does not exists");
			else
				goto oserr;
		}
	} else {
		if (errno == 0)
		error:	irc_privmsg(argv[4], "MOD -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "MOD -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_nam(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	char *args[3], *p;
	struct nick n;
	int nargs = 3, save_errno;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "NAM -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "NAM -- Not logged in");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (nargs > 1 && !(in->in_flags & INF_PRIV)) {
			free(p);
			irc_privmsg(argv[4], "NAM -- Privileged command");
			return (1);
		}

		charset_strtolower(args[0]);
		if (nickdb_find(ircyka_nickdb, args[0], &n) < 0) {
			save_errno = errno;
			free(p);
			if ((errno = save_errno) == ENOENT) {
				irc_privmsg(argv[4],
				    "NAM -- Account does not exist");
				return (1);
			} else
				goto oserr;
		}

		if (nargs < 2) {
			if (n.n_name[0] != '\0')
				irc_privmsg(argv[4], "%s", n.n_name);
			else
				irc_privmsg(argv[4], "NAM -- No name for %s",
				    n.n_nick);
		} else {
			bzero(n.n_name, sizeof(n.n_name));
			(void)strlcpy(n.n_name, args[1], sizeof(n.n_name));
			if (nickdb_update(ircyka_nickdb, &n) < 0) {
				save_errno = errno;
				free(p);
				errno = save_errno;
				goto oserr;
			}
		}
		free(p);
	} else {
		if (errno == 0)
			irc_privmsg(argv[4], "NAM -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "NAM -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_ope(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_channel *ic;
	struct ircyka_join *ij;
	const char *channel = NULL;
	u_int32_t flags;
	int set = 0;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "OPE -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "OPE -- Not logged in");
		return (1);
	}

	if (argv[3] != NULL)
		channel = argv[3];
	else if (argv[4][0] == '#')
		channel = argv[4];

	if (channel != NULL) {
		if ((ic = channel_find(channel)) == NULL) {
			irc_privmsg(argv[4], "OPE -- Not such channel");
			return (1);
		}
		LIST_FOREACH(ij, &in->in_joins, ij_nentry)
			if (ij->ij_ic == ic)
				break;
		if (ij == NULL) {
			irc_privmsg(argv[4], "OPE -- You're not joined %s",
			    ic->ic_channel);
			return (1);
		}
		if (in->in_flags & INF_PRIV)
			flags = AF_OPER;
		else if (acldb_get(ircyka_acldb, ic->ic_channel, in->in_user,
		    &flags) < 0) {
			if (errno == ENOENT)
				irc_privmsg(argv[4],
				    "OPE -- Privilege violation");
			else
			error:	irc_privmsg(argv[4], "OPE -- %s",
				    strerror(errno));
			return (1);
		}
		(void)irc_mode(in->in_nick, ic->ic_channel, flags);
		return (1);
	}

	LIST_FOREACH(ij, &in->in_joins, ij_nentry) {
		if (acldb_get(ircyka_acldb, ij->ij_ic->ic_channel, in->in_user,
		    &flags) < 0) {
			if (errno != ENOENT)
				goto error;
			continue;
		}
		(void)irc_mode(in->in_nick, ij->ij_ic->ic_channel, flags);
		set++;
	}

	if (set == 0)
		irc_privmsg(argv[4],
		    "OPE -- No ACLs found for joined channels");

	return (1);
}

static int
cb_par(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_channel *ic;
	const char *channel;
	u_int32_t flags;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "PAR -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "PAR -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "PAR -- Privileged command");
		return (1);
	}

	if (argv[3] == NULL) {
		if (argv[1][0] != '#') {
			irc_privmsg(argv[4], "PAR -- Channel name required");
			return (1);
		}
		channel = argv[1];
	} else {
		if (argv[3][0] != '#') {
			irc_privmsg(argv[4], "PAR -- Syntax error");
			return (1);
		}
		channel = argv[3];
	}

	if ((ic = channel_find(channel)) == NULL || ic->ic_count == 0) {
		irc_privmsg(argv[4], "PAR -- Not joined %s", channel);
		return (1);
	}

	flags = ic->ic_flags & ICF_PERSIST;
	ic->ic_flags &= ~ICF_PERSIST;
	if (irc_part(ic, irc_nick, irc_partmsg) < 0) {
		irc_privmsg(argv[4], "PAR -- %s", strerror(errno));
		ic->ic_flags |= flags;
	}

	return (1);
}

static int
cb_psw(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct nick n;
	char nick[IRCYKA_MAX_NICKLEN];
	int rc;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "PSW -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "PSW -- Not logged in");
		return (1);
	}

	if (argv[3] == NULL) {
		irc_privmsg(argv[4], "PSW -- Syntax error");
		return (1);
	}

	(void)strlcpy(nick, in->in_user, sizeof(nick));
	if ((rc = nickdb_find(ircyka_nickdb, in->in_user, &n)) == 0) {
		nickdb_passwd(argv[3], n.n_passwd);
		if ((rc = nickdb_update(ircyka_nickdb, &n)) == 0 &&
		    (rc = nickdb_sync(ircyka_nickdb)) == 0)
			return (1);
	}

	if (rc == 1)
		irc_privmsg(argv[4], "PSW -- Nick not found");
	else
		irc_privmsg(argv[4], "PSW -- %s", strerror(errno));
	return (1);
}

static int
cb_set(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	const char *nick, *aflags;
	char *args[3], *p;
	int nargs = 3, ch, self;
	u_int32_t flags, priv;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "SET -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "SET -- Not logged in");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if (nargs > 1) {
			nick = args[0];
			aflags = args[1];
		} else {
			nick = argv[0];
			aflags = args[0];
		}

		priv = in->in_flags & INF_PRIV;
		self = charset_strcmp(nick, in->in_nick) == 0;
		if (!priv && !self) {
		eperm:	free(p);
			irc_privmsg(argv[4], "SET -- Privileged command");
			return (1);
		}

		if ((in = nick_find(nick)) == NULL) {
			free(p);
			irc_privmsg(argv[4], "SET -- Nick not found");
			return (1);
		}

		if (!(in->in_flags & INF_LOGON)) {
			free(p);
			irc_privmsg(argv[4], "SET -- Nick not logged in");
			return (1);
		}

		flags = in->in_flags;
		if ((ch = nick_flags(aflags, &flags)) != 0) {
			free(p);
			if (ch < 0)
				goto error;
			irc_privmsg(argv[4], "SET -- Unsupported flag - %c",
			    ch);
			return (1);
		}

		if (self && !priv && (((flags & INF_CHANOUT) &&
		    !(in->in_flags & INF_CHANOUT) ) || (flags & INF_PRIV)))
			goto eperm;
		else
			in->in_flags = flags;

		if (!(in->in_flags & INF_LOGON))
			nick_logout(in);

		free(p);
	} else {
		if (errno == 0)
		error:	irc_privmsg(argv[4], "SET -- Syntax error");
		else
			irc_privmsg(argv[4], "SET -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_unl(int argc, char * const argv[])
{
	struct ircyka_nick *in;
	struct ircyka_module *im;
	char *args[3], *p;
	int nargs = 3;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "UNL -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "UNL -- Not logged in");
		return (1);
	}

	if (!(in->in_flags & INF_PRIV)) {
		irc_privmsg(argv[4], "UNL -- Privileged command");
		return (1);
	}

	if ((p = match_cmd_parse(&nargs, args, argv[3])) != NULL) {
		if ((im = module_find(args[0])) == NULL) {
			free(p);
			irc_privmsg(argv[4], "UNL -- Module not loaded");
			return (1);
		}

		if (module_unload(im, argv[4], args[1]) < 0) {
			int save_errno = errno;

			free(p);
			errno = save_errno;
			goto oserr;
		}
		free(p);
	} else {
		if (errno == 0)
			irc_privmsg(argv[4], "UNL -- Syntax error");
		else
		oserr:	irc_privmsg(argv[4], "UNL -- %s", strerror(errno));
	}

	return (1);
}

static int
cb_ver(int argc, char * const argv[])
{
	irc_privmsg(argv[4], "%s", ircyka_ident);
	return (1);
}

static int
cb_who(int argc, char * const argv[])
{
	struct ircyka_channel *ic = NULL;
	const char *channel = NULL;
	struct ircyka_nick *in;
	struct ircyka_join *ij;
	int aflag = 0;

	if ((in = nick_find(argv[0])) == NULL) {
		irc_privmsg(argv[4], "WHO -- Who are you?");
		return (1);
	}

	if (!(in->in_flags & INF_LOGON)) {
		irc_privmsg(argv[4], "WHO -- Not logged in");
		return (1);
	}

	if (argv[3] != NULL) {
		if (strcmp(argv[3], "-a") == 0)
			aflag++;
		else if (argv[3][0] == '#')
			channel = argv[3];
		else if (strncmp(argv[3], "-a #", 4) == 0) {
			aflag++;
			channel = argv[3] + 3;
		} else {
			irc_privmsg(argv[4], "WHO -- Syntax error");
			return (1);
		}
		if (!(in->in_flags & INF_PRIV)) {
			irc_privmsg(argv[4], "WHO -- Privileged command");
			return (1);
		}
	}

	if (channel != NULL) {
		if ((ic = channel_find(channel)) == NULL) {
			irc_privmsg(argv[4], "WHO -- No such channel");
			return (1);
		}
		if (LIST_EMPTY(&ic->ic_joins)) {
			irc_privmsg(argv[4], "WHO -- No nicks joined %s",
			    ic->ic_channel);
			return (1);
		}
	}

	irc_privmsg(argv[4], "Nick             Account          "
	    "Time                 Flags Host");
	irc_privmsg(argv[4], "---------------- ---------------- "
	    "-------------------- ----- --------");

	if (channel != NULL) {
		char ts[21];

		LIST_FOREACH(ij, &ic->ic_joins, ij_centry) {
			if (!(ij->ij_in->in_flags & INF_LOGON) && !aflag)
				continue;
			(void)strftime(ts, sizeof(ts), "%d-%b-%Y %H:%M:%S",
			    localtime(&ij->ij_in->in_time));
			irc_privmsg(argv[4], "%-16s %-16s %20s %-5s %s",
			    ij->ij_in->in_nick, ij->ij_in->in_user, ts,
			    nick_aflags(ij->ij_in->in_flags),
			    ij->ij_in->in_host);
		}
		return (1);
	}

	RB_FOREACH(in, ircyka_nick_tree, &ircyka_nicks) {
		char ts[21];

		if (!(in->in_flags & INF_LOGON) && !aflag)
			continue;
		(void)strftime(ts, sizeof(ts), "%d-%b-%Y %H:%M:%S",
		    localtime(&in->in_time));
		irc_privmsg(argv[4], "%-16s %-16s %20s %-5s %s",
		    in->in_nick, in->in_user, ts, nick_aflags(in->in_flags),
		    in->in_host);
	}

	return (1);
}

int
match_cmd_init(void)
{
	RB_INIT(&ircyka_cmd_handlers);

	if (cmd_register("ABO", cb_abo) == NULL ||
	    cmd_register("ACL", cb_acl) == NULL ||
	    cmd_register("ADD", cb_add) == NULL ||
	    cmd_register("BRO", cb_bro) == NULL ||
	    cmd_register("BYE", cb_bye) == NULL ||
	    cmd_register("DEL", cb_del) == NULL ||
	    cmd_register("DIE", cb_die) == NULL ||
	    cmd_register("EXE", cb_exe) == NULL ||
	    cmd_register("HEL", cb_hel) == NULL ||
	    cmd_register("INV", cb_inv) == NULL ||
	    cmd_register("JOI", cb_joi) == NULL ||
	    cmd_register("LOA", cb_loa) == NULL ||
	    cmd_register("LOG", cb_log) == NULL ||
	    cmd_register("LST", cb_lst) == NULL ||
	    cmd_register("MLS", cb_mls) == NULL ||
	    cmd_register("MOD", cb_mod) == NULL ||
	    cmd_register("NAM", cb_nam) == NULL ||
	    cmd_register("OPE", cb_ope) == NULL ||
	    cmd_register("PAR", cb_par) == NULL ||
	    cmd_register("PSW", cb_psw) == NULL ||
	    cmd_register("SET", cb_set) == NULL ||
	    cmd_register("UNL", cb_unl) == NULL ||
	    cmd_register("VER", cb_ver) == NULL ||
	    cmd_register("WHO", cb_who) == NULL)
		return (-1);

	return (0);
}

void
match_cmd(const char *nick, const char *target, const char *cmd,
    const char *args)
{
	struct ircyka_handler *ih;
	struct ircyka_callback *icb;
	struct ircyka_nick *in;
	char *argv[5];

	if ((in = nick_find(nick)) != NULL &&
	    (in->in_flags & INF_CHANOUT))
		bcopy(&target, &argv[4], sizeof(&argv[1]));
	else
		bcopy(&nick, &argv[4], sizeof(&argv[1]));
	bcopy(&nick, &argv[0], sizeof(&argv[0]));
	bcopy(&target, &argv[1], sizeof(&argv[1]));
	bcopy(&cmd, &argv[2], sizeof(&argv[2]));
	bcopy(&args, &argv[3], sizeof(&argv[3]));
	if ((ih = handler_find(&ircyka_cmd_handlers, cmd)) != NULL)
		LIST_FOREACH(icb, &ih->ih_callbacks, icb_entry)
			if (icb->icb_cb(args == NULL ? 3 : 4, argv) != 0)
				break;
}

char *
match_cmd_parse(int *argc, char **argv, const char *s)
{
	char **ap, *str, *p;
	int nargs;

	if ((nargs = *argc - 1) <= 0) {
		errno = EINVAL;
		return (NULL);
	}

	if (s == NULL) {
		*argc = 0;
		argv[0] = NULL;
		errno = 0;
		return (NULL);
	}

	if ((p = str = strdup(s)) != NULL) {
		for (ap = argv; ap < &argv[nargs] &&
		    (*ap = strsep(&str, " ")) != NULL;)
			if (**ap != '\0')
				ap++;
		*ap = NULL;
		*argc = (int)(ap - argv);
		if (str != NULL) {
			while (*--str != '\0')
				;
			*str = ' ';
		}
	}

	return (p);
}
