/*	$RuOBSD$	*/
/*	$OpenBSD: pflogd.c,v 1.9 2001/12/01 23:27:23 miod Exp $	*/

/*
 * Copyright (c) 2001 Theo de Raadt
 * Copyright (c) 2001 Can Erkin Acar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <netinet/ip6.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pcap-int.h>
#include <pcap.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#ifdef __OpenBSD__
#include <util.h>
#endif

#define PCAP_TO_MS	500
#define PCAP_NUM_PKTS	1000
#define PCAP_OPT_FIL	0

#define SNAPLEN		96
#define FLUSH_DELAY	60

#define FILENAME_PREFIX		"/var/log/giflogd."
#define PIDFILE_PREFIX		"giflogd-"
#define DEFAULT_INTERFACE	"gif0"

pcap_t *hpcap;

int Debug = 0;
int snaplen = SNAPLEN;
int gotsig_close, gotsig_alrm, gotsig_hup;

char *filename = NULL;
char *interface = DEFAULT_INTERFACE;
char *filter = 0;

char errbuf[PCAP_ERRBUF_SIZE];

int log_debug = 0;
int delay = FLUSH_DELAY;

FILE *fp;

#ifndef __OpenBSD__
int     pidfile __P((const char *));
#endif

char *copy_argv(char * const *argv);
void logmsg(int priority, const char *message, ...);

char *
copy_argv(char * const *argv)
{
	int len = 0, n;
	char *buf;

	if (argv == NULL)
		return NULL;

	for (n = 0; argv[n]; n++)
		len += strlen(argv[n])+1;
	if (len <= 0)
		return NULL;

	buf = malloc(len);
	if (buf == NULL)
		return NULL;

	strlcpy(buf, argv[0], len);
	for (n = 1; argv[n]; n++) {
		strlcat(buf, " ", len);
		strlcat(buf, argv[n], len);
	}
	return buf;
}

void
logmsg(int pri, const char *message, ...)
{
	va_list ap;
	va_start(ap, message);

	if (log_debug)
		vfprintf(stderr,message,ap);
	else
		vsyslog(pri,message,ap);
	va_end(ap);
}

char *
inet6_ntoa(struct in6_addr in6)
{
	static char buf[NI_MAXHOST];
	struct sockaddr_in6 sa6;
	int sa6len = sizeof(struct sockaddr_in6);

	sa6.sin6_len = sa6len;
	sa6.sin6_family = AF_INET6;
	memcpy(&sa6.sin6_addr, &in6, sizeof(struct in6_addr));
	if (getnameinfo((struct sockaddr *)&sa6, sa6len, buf, NI_MAXHOST,
	    NULL, 0, NI_NUMERICHOST) == 0)
		return buf;
	else
		return "?";
}

void
usage(void)
{
	fprintf(stderr, "usage: giflogd [-hD] [-d delay] [-f filename] ");
	fprintf(stderr, "[-s snaplen] [-i interface] [expression]\n");
	exit(1);
}

void
sig_close(int signal)
{
	gotsig_close = signal;
}

void
sig_hup(int signal)
{
	gotsig_hup = 1;
}

void
sig_alrm(int signal)
{
	gotsig_alrm = 1;
}

int
init_pcap(void)
{
	struct bpf_program bprog;
	pcap_t *oldhpcap = hpcap;

	hpcap = pcap_open_live(interface, snaplen, 1, PCAP_TO_MS, errbuf);
	if (hpcap == NULL) {
		logmsg(LOG_ERR, "Failed to initialize: %s\n",errbuf);
		hpcap = oldhpcap;
		return (-1);
	}

	if (filter) {
		if (pcap_compile(hpcap, &bprog, filter, PCAP_OPT_FIL, 0) < 0)
			logmsg(LOG_WARNING, "%s", pcap_geterr(hpcap));
		else if (pcap_setfilter(hpcap, &bprog) < 0)
			logmsg(LOG_WARNING, "%s", pcap_geterr(hpcap));
	}

	if (oldhpcap)
		pcap_close(oldhpcap);

	snaplen = pcap_snapshot(hpcap);
	logmsg(LOG_NOTICE, "Listening on %s, logging to %s, snaplen %d\n",
		interface, filename, snaplen);
	return (0);
}

int
reset_dump(void)
{
	if (hpcap == NULL)
		return 1;
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}

	fp = fopen(filename, "a+");
	if (fp == NULL) {
		logmsg(LOG_ERR, "Failed to open file %s: %s\n", filename,
		    strerror(errno));
		return 1;
	}

	(void) fseek(fp, 0L, SEEK_END);
	return (0);
}

void
dump_packet(u_char *user, struct pcap_pkthdr *h, u_char *sp)
{
	struct ip6_hdr *ip6;
	int len;
	char src[NI_MAXHOST], dst[NI_MAXHOST];

	ip6 = (struct ip6_hdr *)(sp + 4);
	len = sizeof(struct ip6_hdr) + ntohs(ip6->ip6_plen);
	strcpy(src, inet6_ntoa(ip6->ip6_src));
	strcpy(dst, inet6_ntoa(ip6->ip6_dst));

	fprintf(fp, "ip6 %d %s %s\n", len, src, dst);
}

int
main(int argc, char *argv[])
{
	struct pcap_stat pstat;
	int ch, np;
	char *pidfilename;

	while ((ch = getopt(argc, argv, "hDd:s:f:i:")) != -1) {
		switch (ch) {
		case 'D':
			Debug = 1;
			break;
		case 'd':
			delay = atoi(optarg);
			if (delay < 5 || delay > 60*60)
				usage();
			break;
		case 'f':
			filename = optarg;
			break;
		case 's':
			snaplen = atoi(optarg);
			if (snaplen <= 0)
				snaplen = SNAPLEN;
			break;
		case 'i':
			interface = optarg;
			break;
		case 'h':
		default:
			usage();
		}

	}

	log_debug = Debug;
	argc -= optind;
	argv += optind;

	if (filename == NULL) {
		filename = (char *)malloc(strlen(FILENAME_PREFIX) +
		    strlen(interface) + 1);
		if (filename == NULL) {
			logmsg(LOG_ERR, "malloc(): %s", strerror(errno));
			exit(1);
		}
		sprintf(filename, FILENAME_PREFIX"%s", interface);
	}

	if (!Debug) {
		openlog("giflogd", LOG_PID | LOG_CONS, LOG_DAEMON);
		if (daemon(0, 0)) {
			logmsg(LOG_ERR, "Failed to become daemon: %s",
				strerror(errno));
			exit(1);
		}
		pidfilename = (char *)malloc(strlen(PIDFILE_PREFIX) +
		    strlen(interface) + 1);
		if (pidfilename == NULL) {
			logmsg(LOG_ERR, "malloc()");
			exit(1);
		}
		sprintf(pidfilename, PIDFILE_PREFIX"%s", interface);
		if(pidfile(pidfilename)) {
			logmsg(LOG_ERR, "Failed to create pidfile");
			exit(1);
		}
	}

	(void)umask(S_IRWXG | S_IRWXO);

	signal(SIGTERM, sig_close);
	signal(SIGINT, sig_close);
	signal(SIGQUIT, sig_close);
	signal(SIGALRM, sig_alrm);
	signal(SIGHUP, sig_hup);
	alarm(delay);

	if (argc) {
		filter = copy_argv(argv);
		if (filter == 0)
			logmsg(LOG_NOTICE, "Failed to form filter expression");
	}

	if (init_pcap()) {
		logmsg(LOG_ERR, "Exiting, init failure\n");
		exit(1);
	}

	if (reset_dump()) {
		logmsg(LOG_ERR, "Failed to open log file %s\n", filename);
		pcap_close(hpcap);
		exit(1);
	}

	while (1) {
		np = pcap_dispatch(hpcap, PCAP_NUM_PKTS,
		    (pcap_handler)dump_packet, NULL);
		if (np < 0)
			logmsg(LOG_NOTICE, "%s\n",pcap_geterr(hpcap));

		if (gotsig_close)
			break;
		if (gotsig_hup) {
			if (reset_dump()) {
				logmsg(LOG_ERR, "Failed to open log file!\n");
				break;
			}
			logmsg(LOG_NOTICE, "Reopened logfile\n");
			gotsig_hup = 0;
		}

		if (gotsig_alrm) {
			if (fp != NULL)
				fflush(fp);		/* XXX */
			gotsig_alrm = 0;
			alarm(delay);
		}
	}

	logmsg(LOG_NOTICE, "Exiting due to signal %d\n", gotsig_close);
	if (fp != NULL)
		fclose(fp);

	if (pcap_stats(hpcap, &pstat) < 0)
		logmsg(LOG_WARNING, "Reading stats: %s\n", pcap_geterr(hpcap));
	else
		logmsg(LOG_NOTICE, "%d packets received, %d dropped\n",
		    pstat.ps_recv, pstat.ps_drop);

	pcap_close(hpcap);
	if (!Debug)
		closelog();
	return 0;
}
