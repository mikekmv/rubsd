/*	$RuOBSD: trafd.c,v 1.4 2003/05/16 13:00:36 form Exp $	*/

/*
 * Copyright (c) 2003 Oleg Safiullin <form@pdp11.org.ru>
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
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif
#include <netinet/in.h>
#include <err.h>
#include <errno.h>
#include <pcap.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "trafd.h"
#include "collect.h"
#include "datalinks.h"

sigset_t allsigs;
static int debug;
static int Oflag;
static int pflag;
static pcap_t *pd;

extern char *__progname;

int main(int, char **);
__dead static void usage(void);
static char *copy_argv(char **);
static void sighandler(int);
static int is_running(void);
static void mk_pidfile(void);
static void rm_pidfile(void);

int
main(int argc, char **argv)
{
	char *filter, ebuf[PCAP_ERRBUF_SIZE];
	struct passwd *pw;
	struct bpf_program bprog;
	pcap_handler handler;
	int i;

	while ((i = getopt(argc, argv, "di:Op")) != -1)
		switch (i) {
		case 'd':
			debug = 1;
			break;
		case 'i':
			device = optarg;
			break;
		case 'O':
			Oflag = 1;
			break;
		case 'p':
			pflag = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	if ((pw = getpwnam(TRAFD_USER)) == NULL)
		errx(1, "%s: Unknown user", TRAFD_USER);
	if (pw->pw_dir == NULL || pw->pw_dir[0] == '\0')
		errx(1, "Invalid home directory for %s", pw->pw_name);

	if (device == NULL && (device = pcap_lookupdev(ebuf)) == NULL)
		errx(1, "%s", ebuf);
	if ((pd = pcap_open_live(device, SNAPLEN, !pflag, 1000, ebuf)) == NULL)
		errx(1, "%s", ebuf);

	openlog(__progname, LOG_NDELAY | LOG_PID | (debug ? LOG_PERROR : 0),
	    LOG_DAEMON);

	if (initgroups(pw->pw_name, pw->pw_gid) < 0)
		err(1, "initgroups");
	if (chroot(pw->pw_dir) < 0 || chdir("/") < 0)
		err(1, "chroot: %s", pw->pw_dir);

	(void)seteuid(pw->pw_uid);
	(void)setuid(pw->pw_uid);

	if (is_running())
		errx(1, "Already collecting on %s", device);

	filter = copy_argv(argv + optind);
	if (pcap_compile(pd, &bprog, filter, Oflag, 0) < 0 ||
	    pcap_setfilter(pd, &bprog) < 0)
		errx(1, "%s", pcap_geterr(pd));
	pcap_freecode(&bprog);
	if (filter != NULL)
		free(filter);

	i = pcap_datalink(pd);
	if ((handler = lookup_datalink_handler(i)) == NULL)
		errx(1, "Unknown datalink type %d for interface %s",
		    i, device);

	if (ic_init() < 0)
		err(1, "ic_init");

	if (!debug && daemon(0, 0) < 0)
		err(1, "daemon");

	mk_pidfile();

	sigfillset(&allsigs);
	(void)signal(SIGINFO, sighandler);
	(void)signal(SIGTERM, sighandler);
	(void)signal(SIGINT, sighandler);
	(void)signal(SIGQUIT, sighandler);
	(void)signal(SIGHUP, sighandler);

	setproctitle("collecting traffic on %s", device);
	syslog(LOG_INFO, "traffic collector started on %s", device);

	if (pcap_loop(pd, -1, handler, NULL) < 0)
		syslog(LOG_ERR, "pcap_loop: %s", pcap_geterr(pd));
	pcap_close(pd);

	return (1);
}

__dead static void
usage(void)
{
	(void)fprintf(stderr, "usage: %s [-dOp] [-i interface] [expression]\n",
	    __progname);
	exit(1);
}

static char *
copy_argv(char **argv)
{
	int i, len = 0;
	char *buf;

	if (argv == NULL)
		return (NULL);

	for (i = 0; argv[i] != NULL; i++)
		len += strlen(argv[i]) + 1;
	if (len == 0)
		return (NULL);

	if ((buf = malloc(len)) == NULL)
		err(1, "copy_argv");

	(void)strlcpy(buf, argv[0], len);
	for (i = 1; argv[i] != NULL; i++) {
		(void)strlcat(buf, " ", len);
		(void)strlcat(buf, argv[i], len);
	}
	return (buf);
}

static void
sighandler(int signo)
{
	struct pcap_stat ps;

	sigprocmask(SIG_BLOCK, &allsigs, NULL);

	if (signo != SIGHUP) {
		if (pcap_stats(pd, &ps) < 0)
			syslog(LOG_ERR, "pcap_stats: %s", pcap_geterr(pd));
		else
			syslog(ps.ps_drop ? LOG_WARNING : LOG_INFO,
			    "%u packets received, %u packets dropped",
			    ps.ps_recv, ps.ps_drop);
	}

	if (signo != SIGINFO && ic_dump(device) < 0)
		syslog(LOG_ERR, "ic_dump: %m");

	if (signo != SIGHUP && signo != SIGINFO) {
		pcap_close(pd);
		rm_pidfile();
		syslog(LOG_INFO, "traffic collector stopped on %s", device);
		_exit(0);
	}
}

int
is_running(void)
{
	char file[MAXPATHLEN];
	unsigned long u;
	char *ep;
	FILE *fp;

	(void)snprintf(file, sizeof(file), TRAFD_PIDFILE, device);
	if ((fp = fopen(file, "r")) == NULL) {
		if (errno == ENOENT)
			return (0);
		err(1, "is_running: %s", file);
	}
	ep = fgets(file, sizeof(file), fp);
	(void)fclose(fp);
	if (ep == 0)
		return (0);

	errno = 0;
	u = strtoul(file, &ep, 10);
	if (*ep != '\n' || errno == ERANGE)
		return (0);
	return (kill((pid_t)u, 0) == 0);
}

static void
mk_pidfile(void)
{
	char file[MAXPATHLEN];
	FILE *fp;

	if (snprintf(file, sizeof(file), TRAFD_PIDFILE, device) >=
	    sizeof(file)) {
		syslog(LOG_WARNING, "mk_pidfile: %s: %s", file,
		    strerror(ENAMETOOLONG));
		return;
	}
	if ((fp = fopen(file, "w+")) == NULL) {
	error:	syslog(LOG_WARNING, "mk_pidfile: %s: %m", file);
	exit:	(void)fclose(fp);
		return;
	}
	if (fprintf(fp, "%u\n", getpid()) < 0)
		goto error;

	goto exit;
}

static void
rm_pidfile(void)
{
	char file[MAXPATHLEN];

	if (snprintf(file, sizeof(file), TRAFD_PIDFILE, device) >=
	    sizeof(file)) {
		errno = ENAMETOOLONG;
	error:	syslog(LOG_WARNING, "rm_pidfile: %s: %m", file);
		return;
	}

	if (unlink(file) < 0)
		goto error;
}
