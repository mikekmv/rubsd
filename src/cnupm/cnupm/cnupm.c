/*	$RuOBSD: cnupm.c,v 1.15 2004/04/19 12:53:41 form Exp $	*/

/*
 * Copyright (c) 2003 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#ifdef HAVE_ERR
#include <err.h>
#endif
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_LOGIN_CAP
#include <login_cap.h>
#endif
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "cnupm.h"
#include "inet6.h"
#include "collect.h"
#include "datalinks.h"

static char *cnupm_interface;
static char *cnupm_user = CNUPM_USER;
static char *cnupm_infile;
static pcap_t *pd;
static int cnupm_debug;
static int quiet_mode;
static int cnupm_pktopt = 1;
static int cnupm_promisc = 1;
static int need_empty_dump;
static int cnupm_terminate;

#define PCAP_TIMEOUT		1000

int main(int, char **);
static void usage(void);
static char *copy_argv(char **);
static char *copy_file(int, const char *);
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
	int ch, fd = -1;

	cnupm_progname(argv);
	while ((ch = getopt(argc, argv, "def:F:i:m:NOpPqu:V")) != -1)
		switch (ch) {
		case 'd':
			cnupm_debug = 1;
			break;
		case 'e':
			need_empty_dump = 1;
			break;
		case 'f':
			collect_family = cnupm_family(optarg);
			if (collect_family == AF_UNSPEC)
				errx(1, "%s: Address family not supported",
				    optarg);
			break;
		case 'F':
			cnupm_infile = optarg;
			break;
		case 'i':
			cnupm_interface = optarg;
			break;
		case 'm':
			ct_entries_max = cnupm_ulval(optarg, MIN_CT_ENTRIES,
			    MAX_CT_ENTRIES);
			if (errno != 0)
				err(1, "%s", optarg);
			break;
		case 'N':
			collect_proto = 0;
			break;
		case 'O':
			cnupm_pktopt = 0;
			break;
		case 'p':
			cnupm_promisc = 0;
			break;
		case 'P':
			collect_ports = 0;
			break;
		case 'q':
			quiet_mode = 1;
			break;
		case 'u':
			cnupm_user = optarg;
			break;
		case 'V':
			cnupm_version(1);
			/* NOTREACHED */
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

	if (cnupm_infile != NULL && (fd = open(cnupm_infile, O_RDONLY)) < 0)
		err(1, "%s", cnupm_infile);

	if (cnupm_restrict(pw) < 0)
		err(1, "cnupm_restrict");

	if ((ch = cnupm_pidfile(CNUPM_PIDFILE_CHECK, CNUPM_PIDFILE,
	    cnupm_interface)) < 0)
		err(1, "cnupm_pidfile");
	if (ch > 0)
		errx(1, "Already collecting on interface %s", cnupm_interface);

	if (cnupm_infile != NULL)
		filter = copy_file(fd, cnupm_infile);
	else
		filter = copy_argv(argv);

	if (pcap_compile(pd, &bprog, filter, cnupm_pktopt, 0) < 0 ||
	    pcap_setfilter(pd, &bprog) < 0)
		errx(1, "%s", pcap_geterr(pd));
#ifdef HAVE_PCAP_FREECODE
	pcap_freecode(&bprog);
#endif
	if (filter != NULL)
		free(filter);

	ch = pcap_datalink(pd);
	if ((datalink_handler = lookup_datalink_handler(ch)) == NULL)
		errx(1, "Unsupported datalink type %d for interface %s", ch,
		    cnupm_interface);

	if (collect_init())
		err(1, "collect_init");

	if (cnupm_daemon(cnupm_debug) < 0)
		err(1, "cnupm_daemon");

	if (cnupm_pidfile(CNUPM_PIDFILE_CREATE, CNUPM_PIDFILE,
	    cnupm_interface) < 0) {
		syslog(LOG_WARNING, "(%s) cnupm_pidfile: %m", cnupm_interface);
		if (cnupm_debug)
			warn("(%s) cnupm_pidfile", cnupm_interface);
	}

	(void)sigprocmask(SIG_SETMASK, &sa.sa_mask, NULL);
	sigfillset(&sa.sa_mask);
#ifdef SA_RESTART
	sa.sa_flags = SA_RESTART;
#else
	sa.sa_flags = 0;
#endif
	sa.sa_handler = cnupm_signal;
	(void)sigaction(SIGHUP, &sa, NULL);
#ifdef SIGINFO
	(void)sigaction(SIGINFO, &sa, NULL);
#endif
	(void)sigaction(SIGUSR1, &sa, NULL);
	(void)sigaction(SIGTERM, &sa, NULL);
	(void)sigaction(SIGINT, &sa, NULL);
	(void)sigaction(SIGQUIT, &sa, NULL);

	setproctitle("collecting traffic on %s", cnupm_interface);
	syslog(LOG_INFO, "(%s) traffic collector started", cnupm_interface);
	if (cnupm_debug)
		warnx("(%s) traffic collector started", cnupm_interface);
	while (!cnupm_terminate) {
		if (pcap_dispatch(pd, 0, datalink_handler, NULL) < 0) {
			syslog(LOG_ERR, "(%s) pcap_dispatch: %s",
			    cnupm_interface, pcap_geterr(pd));
			if (cnupm_debug)
				warnx("(%s) pcap_dispatch: %s",
				    cnupm_interface, pcap_geterr(pd));
			break;
		}
		if (collect_need_dump) {
			int dumped;

			if ((dumped = collect_dump(cnupm_interface,
			    need_empty_dump)) < 0) {
				syslog(LOG_ERR, "(%s) collect_dump: %m",
				    cnupm_interface);
				if (cnupm_debug)
					warn("(%s) collect_dump",
					    cnupm_interface);
				continue;
			}

			if (!quiet_mode && (dumped != 0 || need_empty_dump)) {
				syslog(LOG_INFO,
				    "(%s) %u records dumped to file",
				    cnupm_interface, dumped);
				if (cnupm_debug)
					warnx("(%s) %u records dumped to file",
					    cnupm_interface, dumped);
			}
		}
	}
	log_stats();
	pcap_close(pd);
	(void)cnupm_pidfile(CNUPM_PIDFILE_REMOVE, CNUPM_PIDFILE,
	    cnupm_interface);
	syslog(LOG_INFO, "(%s) traffic collector stopped", cnupm_interface);
	if (cnupm_debug)
		warnx("(%s) traffic collector stopped", cnupm_interface);

	return (!cnupm_terminate);
}

static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: %s [-deNOpPqV] [-f family] [-F file] [-i interface] "
	    "[-m maxentries] [-u user] [expression]\n", __progname);
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

static char *
copy_file(int fd, const char *file)
{
	struct stat st;
	ssize_t nbytes;
	char *cp;

	if (fstat(fd, &st) < 0)
		err(1, "stat: %s", file);
	if ((cp = malloc((size_t)st.st_size + 1)) == NULL)
		err(1, "copy_file");
	if ((nbytes = read(fd, cp, (size_t)st.st_size)) < 0)
		err(1, "read: %s", file);
	if ((size_t)nbytes != st.st_size)
		errx(1, "%s: Short read", file);
	cp[(int)st.st_size] = '\0';
	return (cp);
}

static void
cnupm_signal(int signo)
{
#ifdef SIGINFO
	if (signo == SIGINFO)
		signo = SIGUSR1;
#endif
	if (signo == SIGUSR1)
		log_stats();

	if (signo != SIGUSR1)
		collect_need_dump = 1;

	if (signo != SIGHUP && signo != SIGUSR1)
		cnupm_terminate = 1;
}

static void
log_stats(void)
{
	struct pcap_stat ps;

	if (pcap_stats(pd, &ps) < 0) {
		syslog(LOG_ERR, "(%s) pcap_stats: %s", cnupm_interface,
		    pcap_geterr(pd));
		if (cnupm_debug)
			warnx("(%s) pcap_stats: %s", cnupm_interface,
			    pcap_geterr(pd));
	} else {
		int prio;

		prio = (ps.ps_drop || collect_lost_packets) ?
		    LOG_WARNING : LOG_INFO;
		syslog(prio,
		    "(%s) %u packets received, %u dropped, %u lost",
		    cnupm_interface, ps.ps_recv, ps.ps_drop,
		    collect_lost_packets);
		if (cnupm_debug)
			warnx("(%s) %u packets received, %u dropped, %u lost",
			    cnupm_interface, ps.ps_recv, ps.ps_drop,
			    collect_lost_packets);
	}
}
