/*	$RuOBSD$	*/

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
#ifdef LOGIN_CAP
#include <login_cap.h>
#endif
#include <pcap.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "cnupm.h"
#include "collect.h"
#include "datalinks.h"

extern char *__progname;

static int cnupm_debug;
static int cnupm_pktopt = 1;
static int cnupm_promisc = 1;
static char *cnupm_interface;
static char *cnupm_user = CNUPM_USER;
static int cnupm_terminate;
static pcap_t *pd;

#ifdef __FreeBSD__
#define __dead			__dead2
#endif

#define PCAP_TIMEOUT		1000

int main(int, char **);
__dead static void usage(void);
static char *copy_argv(char **);
static void cnupm_signal(int);
static void log_stats(void);

int
main(int argc, char **argv)
{
	char ebuf[PCAP_ERRBUF_SIZE], *filter;
	pcap_handler datalink_handler;
	struct bpf_program bprog;
	struct sigaction sa;
	struct passwd *pw;
	int ch;

	while ((ch = getopt(argc, argv, "di:Opu:V")) != -1)
		switch (ch) {
		case 'd':
			cnupm_debug = 1;
			break;
		case 'i':
			cnupm_interface = optarg;
			break;
		case 'O':
			cnupm_pktopt = 0;
			break;
		case 'p':
			cnupm_promisc = 0;
			break;
		case 'u':
			cnupm_user = optarg;
			break;
		case 'V':
			(void)fprintf(stderr, "cnupm v%u.%u, libpcap v%u.%u",
			    CNUPM_VERSION_MAJOR, CNUPM_VERSION_MINOR,
			    PCAP_VERSION_MAJOR, PCAP_VERSION_MINOR);
#ifdef INET6
			(void)fprintf(stderr, ", INET6");
#endif
#ifdef PROTO
			(void)fprintf(stderr, ", PROTO");
#endif
#ifdef PORTS
			(void)fprintf(stderr, ", PORTS");
#endif
			(void)fprintf(stderr, "\n");
			return (0);
		default:
			usage();
			/* NOTREACHED */
		}
	argv += optind;

	if ((pw = getpwnam(cnupm_user)) == NULL)
		errx(1, "No passwd entry for %s", cnupm_user);
	if (pw->pw_dir == NULL || pw->pw_dir[0] == '\0')
		errx(1, "No home directory for %s", cnupm_user);

	if (cnupm_interface == NULL &&
	    (cnupm_interface = pcap_lookupdev(ebuf)) == NULL)
		errx(1, "%s", ebuf);
	if ((pd = pcap_open_live(cnupm_interface, CNUPM_SNAPLEN, cnupm_promisc,
	    PCAP_TIMEOUT, ebuf)) == NULL)
		errx(1, "%s", ebuf);

	tzset();
	openlog(__progname, LOG_NDELAY | LOG_PID |
	    (cnupm_debug ? LOG_PERROR : 0), LOG_DAEMON);

#ifdef LOGIN_CAP
	if (setusercontext(NULL, pw, pw->pw_uid,
	    LOGIN_SETALL & ~LOGIN_SETUSER) < 0)
		err(1, "setusercontext");
#else
	if (initgroups(pw->pw_name, pw->pw_gid) < 0)
		err(1, "initgroups");
#endif

	if (chroot(pw->pw_dir) < 0 || chdir("/") < 0)
		err(1, "Can't chroot to %s", pw->pw_dir);

	(void)seteuid(pw->pw_uid);
	(void)setuid(pw->pw_uid);

	if ((ch = cnupm_pidfile(cnupm_interface, CNUPM_PIDFILE_CHECK)) < 0)
		err(1, "cnupm_pidfile");
	if (ch > 0)
		errx(1, "Already running on interface %s", cnupm_interface);

	filter = copy_argv(argv);
	if (pcap_compile(pd, &bprog, filter, cnupm_pktopt, 0) < 0 ||
	    pcap_setfilter(pd, &bprog) < 0)
		errx(1, "%s", pcap_geterr(pd));
	pcap_freecode(&bprog);
	if (filter != NULL)
		free(filter);

	ch = pcap_datalink(pd);
	if ((datalink_handler = lookup_datalink_handler(ch)) == NULL)
		errx(1, "Unsupported datalink type %d for interface %s", ch,
		    cnupm_interface);

	if (collect_init())
		err(1, "collect_init");

	if (!cnupm_debug && daemon(0, 0) < 0)
		err(1, "daemon");

	if (cnupm_pidfile(cnupm_interface, CNUPM_PIDFILE_CREATE) < 0)
		syslog(LOG_WARNING, "(%s) cnupm_pidfile: %m", cnupm_interface);

	sigfillset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = cnupm_signal;
	(void)sigaction(SIGHUP, &sa, NULL);
	(void)sigaction(SIGINFO, &sa, NULL);
	(void)sigaction(SIGTERM, &sa, NULL);
	(void)sigaction(SIGINT, &sa, NULL);
	(void)sigaction(SIGQUIT, &sa, NULL);

#ifdef HAVE_SETPROCTITLE
	setproctitle("collecting traffic on %s", cnupm_interface);
#endif
	syslog(LOG_INFO, "(%s) traffic collector started", cnupm_interface);
	for (;;) {
		if (pcap_dispatch(pd, 0, datalink_handler, NULL) < 0) {
			syslog(LOG_ERR, "(%s) pcap_loop: %s", cnupm_interface,
			    pcap_geterr(pd));
			break;
		}
		if (collect_need_dump) {
			int dumped;

			if ((dumped = collect_dump(cnupm_interface)) < 0) {
				syslog(LOG_ERR, "(%s) collect_dump: %m",
				    cnupm_interface);
				continue;
			}
#ifndef NEED_EMPTY_DUMP
			if (dumped != 0)
#endif
				syslog(LOG_INFO,
				    "(%s) %u records dumped to file",
				    cnupm_interface, dumped);
		}
		if (cnupm_terminate)
			break;
	}
	log_stats();
	pcap_close(pd);
	(void)cnupm_pidfile(cnupm_interface, CNUPM_PIDFILE_REMOVE);
	syslog(LOG_INFO, "(%s) traffic collector stopped",
	    cnupm_interface);

	return (!cnupm_terminate);
}

static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: %s [-dOpV] [-i interface] [-u user] [expression]\n",
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
cnupm_signal(int signo)
{
	if (signo == SIGINFO)
		log_stats();

	if (signo != SIGINFO)
		collect_need_dump = 1;

	if (signo != SIGHUP && signo != SIGINFO)
		cnupm_terminate = 1;
}

static void
log_stats(void)
{
	struct pcap_stat ps;

	if (pcap_stats(pd, &ps) < 0)
		syslog(LOG_ERR, "(%s) pcap_stats: %s", cnupm_interface,
		    pcap_geterr(pd));
	else {
		int prio;

		prio = (ps.ps_drop || collect_lost_packets) ?
		    LOG_WARNING : LOG_INFO;
		syslog(prio, "(%s) %u packets received, %u dropped, %u lost",
		    cnupm_interface, ps.ps_recv, ps.ps_drop,
		    collect_lost_packets);
	}
}
