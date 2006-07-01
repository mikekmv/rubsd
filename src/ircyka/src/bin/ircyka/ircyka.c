/*
 * $RuOBSD: ircyka.c,v 1.1.1.1 2006/02/24 17:13:31 form Exp $
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
#include <sys/socket.h>
#include <sys/utsname.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <login_cap.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "ircyka/ircyka.h"
#include "ircyka/nickdb.h"
#include "ircyka/acldb.h"


char *ircyka_conf_name = IRCYKA_CONF_NAME;
char *ircyka_conf_file;
char *ircyka_conf_user;
char *ircyka_conf_dir;
char *ircyka_ident;
char *ircyka_nickdb_file;
char *ircyka_acldb_file;
char *ircyka_log_target;
void *ircyka_nickdb;
void *ircyka_acldb;
int ircyka_debug;
int ircyka_delay;

char *irc_host;
char *irc_port;
char *irc_user;
char *irc_name;
char *irc_nick;
char *irc_pass;
char *irc_domain;
char *irc_hostname;
char *irc_servname;
char *irc_channels;
char *irc_charset;
char *irc_partmsg;
char *irc_quitmsg;
char *irc_killmsg;
char *irc_bitchmsg;
char *irc_reconnect;
char *log_target;
char *mod_load;
int irc_joinall;
int irc_bitch;

static struct ircyka_conf_var vars[] = {
	ICV_STR("irc_host", irc_host, IRCYKA_DEF_HOST),
	ICV_STR("irc_port", irc_port, IRCYKA_DEF_PORT),
	ICV_STR("irc_user", irc_user, NULL),
	ICV_STR("irc_name", irc_name, NULL),
	ICV_STR("irc_nick", irc_nick, NULL),
	ICV_STR("irc_pass", irc_pass, NULL),
	ICV_STR("irc_domain", irc_domain, NULL),
	ICV_STR("irc_hostname", irc_hostname, NULL),
	ICV_STR("irc_servname", irc_servname, NULL),
	ICV_STR("irc_channels", irc_channels, NULL),
	ICV_STR("irc_charset", irc_charset, IRCYKA_DEF_CHARSET),
	ICV_STR("irc_partmsg", irc_partmsg, NULL),
	ICV_STR("irc_quitmsg", irc_quitmsg, NULL),
	ICV_STR("irc_killmsg", irc_killmsg, NULL),
	ICV_STR("irc_reconnect", irc_reconnect, 0),
	ICV_STR("irc_bitch", irc_bitchmsg, NULL),
	ICV_BOOL("irc_bitch", irc_bitch, 0),
	ICV_BOOL("irc_joinall", irc_joinall, 0),
	ICV_STR("log_target", log_target, NULL),
	ICV_STR("mod_load", mod_load, NULL),
	ICV_EOV
};


int main(int, char *const *);
__dead static void usage(void);
static void ircyka_autoload(void);
static void ircyka_signal(int);


int
main(int argc, char *const argv[])
{
	struct sigaction sa;
	struct utsname un;
	const char *errstr;
	struct passwd *pw;
	uid_t uid;
	int ch, s;

	(void)setlocale(LC_ALL, "C");

	while ((ch = getopt(argc, argv, "c:du:")) != -1)
		switch (ch) {
		case 'c':
			ircyka_conf_file = optarg;
			break;
		case 'd':
			ircyka_debug++;
			break;
		case 'u':
			ircyka_conf_user = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	if (argc > 1)
		usage();
	if (argc == 1)
		ircyka_conf_name = argv[0];

	if (uname(&un) < 0)
		err(EX_UNAVAILABLE, "uname");

#ifdef IRCYKA_VERSION_PATCH
	if (asprintf(&ircyka_ident, "IRCyka %u.%u.%u [%s/%s %s]",
	    IRCYKA_VERSION_MAJOR, IRCYKA_VERSION_MINOR,
	    IRCYKA_VERSION_PATCH, un.sysname, un.machine, un.release) < 0)
		err(EX_UNAVAILABLE, NULL);
#else
	if (asprintf(&ircyka_ident, "IRCyka %u.%u [%s/%s %s]",
	    IRCYKA_VERSION_MAJOR, IRCYKA_VERSION_MINOR,
	    un.sysname, un.machine, un.release) < 0)
		err(EX_UNAVAILABLE, NULL);
#endif

	if ((uid = geteuid()) == 0) {
		if (ircyka_conf_user == NULL)
			ircyka_conf_user = IRCYKA_CONF_USER;
		if ((pw = getpwnam(ircyka_conf_user)) == NULL)
			errx(EX_CONFIG, "No passwd entry for %s",
			    ircyka_conf_user);
		if (pw->pw_dir[0] == '\0')
			errx(EX_CONFIG, "No home directory for %s",
			    ircyka_conf_user);
		if (setusercontext(NULL, pw, pw->pw_uid, LOGIN_SETALL) < 0)
			err(EX_UNAVAILABLE, "setusercontext");
		ircyka_conf_dir = pw->pw_dir;
	} else {
		if (ircyka_conf_user != NULL)
			errx(EX_NOPERM, "%s", strerror(EPERM));
		ircyka_conf_user = IRCYKA_CONF_USER;
		if ((pw = getpwuid(uid)) == NULL)
			errx(EX_CONFIG, "Who are you?");
		if (pw->pw_dir[0] == '\0')
			errx(EX_CONFIG, "No home directory for %s",
			    ircyka_conf_user);
		if (asprintf(&ircyka_conf_dir, "%s/%s", pw->pw_dir,
		    IRCYKA_CONF_DIR) < 0)
			err(EX_UNAVAILABLE, NULL);
	}
	bzero(pw->pw_passwd, strlen(pw->pw_passwd));
	endpwent();

	conf_setdef(vars, "irc_user", ICVT_STR, ircyka_conf_user);
	conf_setdef(vars, "irc_name", ICVT_STR, ircyka_ident);
	conf_setdef(vars, "irc_nick", ICVT_STR, ircyka_conf_user);
	conf_setdef(vars, "irc_domain", ICVT_STR, un.nodename);
	conf_setdef(vars, "irc_hostname", ICVT_STR, un.nodename);
	conf_setdef(vars, "irc_servname", ICVT_STR, ircyka_ident);

	if (ircyka_conf_file == NULL && asprintf(&ircyka_conf_file, "%s/%s",
	    ircyka_conf_dir, IRCYKA_CONF_FILE) < 0)
		err(EX_UNAVAILABLE, NULL);

	if (asprintf(&ircyka_nickdb_file, "%s/%s", ircyka_conf_dir,
	    IRCYKA_NICKDB_FILE) < 0 || asprintf(&ircyka_acldb_file, "%s/%s",
	    ircyka_conf_dir, IRCYKA_ACLDB_FILE) < 0)
		err(EX_UNAVAILABLE, NULL);

	switch (conf_load(ircyka_conf_file, ircyka_conf_name, vars)) {
	case 1:
		errx(EX_CONFIG, "%s: Unresolved tc expansion",
		    ircyka_conf_file);
		/* NOTREACHED */
	case -1:
		errx(EX_CONFIG, "%s: Configuration not found",
		    ircyka_conf_name);
		/* NOTREACHED */
	case -2:
		err(EX_UNAVAILABLE, "%s", ircyka_conf_file);
		/* NOTREACHED */
	case -3:
		errx(EX_CONFIG, "%s: Potential reference loop detected",
		    ircyka_conf_file);
		/* NOTREACHED */
	}

	if (charset_init(irc_charset) < 0)
		conf_reset(vars, "irc_charset", ICVT_STR);

	if (irc_bitchmsg != NULL)
		irc_bitch = 1;

	if (irc_reconnect != NULL) {
		ircyka_delay = strtonum(irc_reconnect, 0, 3600, &errstr);
		if (errstr != NULL) {
			ircyka_delay = 0;
			warnx("irc_reconnect: %s: %s", irc_reconnect, errstr);
		}
	}

	if ((ircyka_nickdb = nickdb_open(ircyka_nickdb_file, 1)) == NULL &&
	    errno != ENOENT)
		err(EX_UNAVAILABLE, "nickdb_open: %s", ircyka_nickdb_file);
	if (ircyka_nickdb == NULL) {
		struct nick n;

		if ((ircyka_nickdb = nickdb_creat(ircyka_nickdb_file)) == NULL)
			err(EX_UNAVAILABLE, "nickdb_creat: %s",
			    ircyka_nickdb_file);
		bzero(&n, sizeof(n));
		(void)strlcpy(n.n_nick, irc_nick, sizeof(n));
		if (irc_pass != NULL)
			nickdb_passwd(irc_pass, n.n_passwd);
		else
			nickdb_passwd(irc_nick, n.n_passwd);
		n.n_flags = NF_PRIV;
		if (nickdb_add(ircyka_nickdb, &n) < 0)
			err(EX_UNAVAILABLE, "nickdb_add");
		(void)nickdb_sync(ircyka_nickdb);
		if (ircyka_debug)
			(void)fprintf(stderr, "Nick database created\n");
	}

	if ((ircyka_acldb = acldb_open(ircyka_acldb_file, 1)) == NULL &&
	    errno != ENOENT)
		err(EX_UNAVAILABLE, "acldb_open: %s", ircyka_acldb_file);
	if (ircyka_acldb == NULL) {
		if ((ircyka_acldb = acldb_creat(ircyka_acldb_file)) == NULL)
			err(EX_UNAVAILABLE, "acldb_creat: %s",
			    ircyka_acldb_file);
		if (ircyka_debug)
			(void)fprintf(stderr, "ACL database created\n");
		(void)acldb_sync(ircyka_acldb);
	}

	if (match_irc_init() < 0)
		err(EX_UNAVAILABLE, "match_irc_init");

	if (match_cmd_init() < 0)
		err(EX_UNAVAILABLE, "match_cmd_init");


	if (!ircyka_debug && daemon(0, 0) < 0)
		err(EX_OSERR, "daemon");

	sigfillset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = ircyka_signal;
	(void)sigaction(SIGHUP, &sa, NULL);
	(void)sigaction(SIGINT, &sa, NULL);
	(void)sigaction(SIGQUIT, &sa, NULL);
	(void)sigaction(SIGTERM, &sa, NULL);
	(void)sigaction(SIGALRM, &sa, NULL);
	(void)sigaction(SIGPIPE, &sa, NULL);

	ircyka_autoload();

	while (!ircyka_die) {
		if ((s = ircyka_connect(&errstr)) < 0) {
			if (s == -1)
				warn("%s", errstr);
			else
				warnx("%s", errstr);
		} else {
			channel_init();
			qio_init(s);
			ircyka_loop(s);
			(void)shutdown(s, SHUT_RDWR);
			(void)close(s);
		}

		if (!ircyka_die && ircyka_delay != 0) {
			if (ircyka_debug)
				(void)fprintf(stderr,
				    "\nSleeping for %d seconds...\n",
				    ircyka_delay);
			sleep(ircyka_delay);
		} else
			break;
	}

	(void)nickdb_close(ircyka_nickdb);
	(void)acldb_close(ircyka_acldb);
	return (ircyka_die ? EX_OK : EX_UNAVAILABLE);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-d] [-c conf_file] [-u user] [conf_name]\n",
	    __progname);
	exit(EX_USAGE);
}

static void
ircyka_autoload(void)
{
	char module[IRCYKA_MAX_MODNAME + 2];
	char *bp, *cp;

	if (mod_load == NULL)
		return;

	bp = cp = mod_load;
	while (cp != NULL) {
		(void)strlcpy(module, cp, sizeof(module));
		if ((cp = strchr(module, ' ')) != NULL) {
			*cp++ = '\0';
			cp = (bp += strlen(module) + 1);
		}
		if (ircyka_debug)
			(void)fprintf(stderr, "Loading module %s...\n", module);
		if (module_load(module, NULL, NULL) == NULL) {
			if (errno == 0)
				warnx("module_load: %s", dlerror());
			else
				warn("module_load");
		}
	}
}

static void
ircyka_signal(int signo)
{
	switch (signo) {
	case SIGALRM:
		ircyka_got_alarm++;
		break;
	case SIGPIPE:
		break;
	default:
		ircyka_die++;
		break;
	}
}
