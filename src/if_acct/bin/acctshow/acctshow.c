/*	$RuOBSD: acctshow.c,v 1.1 2004/10/31 06:51:27 form Exp $	*/

/*
 * Copyright (c) 2004 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_acct.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>

#include "term.h"


#define SORT_LAST	1
#define SORT_SRC	2
#define SORT_DST	3
#define SORT_PKTS	4
#define SORT_OCTETS	5


static char *interface = "acct0";
static int seconds = 1;
static int smode = -1;
static int order = 1;
static int restart;


int main(int, char **);
__dead static void usage(void);
static void sighandler(int);
static int intvalue(const char *);
static int sortmode(const char *);
static int sort_last(const void *, const void *);
static int sort_src(const void *, const void *);
static int sort_dst(const void *, const void *);
static int sort_pkts(const void *, const void *);
static int sort_octets(const void *, const void *);


int
main(int argc, char **argv)
{
	char head[60];
	struct acct_flow af[ACCTFLOWS];
	struct acctio_flows aif = { af, ACCTFLOWS };
	struct acct_flow *paf[ACCTFLOWS];
	struct sigaction sa;
	struct ifreq ifr;
	struct utsname un;
	u_int64_t pkts, octets, opkts = 0, ooctets = 0;
	int ch, s, page, npages, row, nrows, error = 0;

	while ((ch = getopt(argc, argv, "i:o:rs:")) != -1)
		switch (ch) {
		case 'i':
			interface = optarg;
			break;
		case 'o':
			if ((smode = sortmode(optarg)) < 0)
				err(EX_UNAVAILABLE, "-o %s", optarg);
			break;
		case 'r':
			order = -order;
			break;
		case 's':
			if ((seconds = intvalue(optarg)) <= 0)
				err(EX_UNAVAILABLE, "-s %s", optarg);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (smode < 0) {
		smode = SORT_LAST;
		order = -order;
	}

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err(EX_UNAVAILABLE, "socket");

	bzero(&ifr, sizeof(ifr));
	(void)strlcpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	ifr.ifr_data = (caddr_t)&aif;

	if (ioctl(s, SIOCGFLOWS, &ifr) < 0) {
		if (errno != ENXIO)
			err(EX_UNAVAILABLE, "SIOCGFLOWS");
		else {
			error = errno;
			aif.aif_nflows = 0;
		}
	}

rstart:	if (term_open(NULL, seconds) < 0)
		errx(EX_UNAVAILABLE, "Couldn't initialize terminal");
	if (term_nrows() < 5 || term_ncols() < 80) {
		term_close();
		errx(EX_UNAVAILABLE, "Terminal window is too small");
	}

	if (uname(&un) < 0) {
		bzero(&un, sizeof(un));
		un.sysname[0] = un.nodename[0] = un.release[0] = '?';
		un.machine[0] = '?';
	}

	(void)sigfillset(&sa.sa_mask);
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	(void)sigaction(SIGHUP, &sa, NULL);
	(void)sigaction(SIGINT, &sa, NULL);
	(void)sigaction(SIGQUIT, &sa, NULL);
	(void)sigaction(SIGTERM, &sa, NULL);
	(void)sigaction(SIGWINCH, &sa, NULL);

	(void)setlocale(LC_ALL, "C");

redraw:	nrows = term_nrows() - 4;
	(void)snprintf(head, sizeof(head), "%s/%s %s %s (%s)",
	    un.sysname, un.machine, un.release, un.nodename, interface);
	term_printf("%s", head);
	term_setxy(0, 1);
	ch = term_reverse();
	term_printf("LAST                  SRC              DST         "
	    "            PKTS       OCTETS");
	term_setxy(0, nrows + 2);
	if (ch)
		term_printf("%80s", " ");
	else
		for (ch = 0; ch < 8; ch++)
			term_printf("----------");
	term_normal();

	page = 1;
	npages = aif.aif_nflows / nrows + (aif.aif_nflows % nrows != 0);
	for (;;) {
		char tstamp[21];
		time_t tm;

		(void)time(&tm);
		(void)strftime(tstamp, sizeof(tstamp), "%d-%h-%Y %H:%M:%S",
		    localtime(&tm));
		term_setxy(60, 0);
		term_printf("%s", tstamp);

		if (page > npages)
			page = 1;

		for (ch = pkts = octets = 0; ch < aif.aif_nflows; ch++) {
			paf[ch] = &af[ch];
			pkts += aif.aif_flows[ch].af_pkts;
			octets += aif.aif_flows[ch].af_octets;
		}
		switch (smode) {
		case SORT_LAST:
			qsort(paf, aif.aif_nflows, sizeof(*paf), sort_last);
			break;
		case SORT_SRC:
			qsort(paf, aif.aif_nflows, sizeof(*paf), sort_src);
			break;
		case SORT_DST:
			qsort(paf, aif.aif_nflows, sizeof(*paf), sort_dst);
			break;
		case SORT_PKTS:
			qsort(paf, aif.aif_nflows, sizeof(*paf), sort_pkts);
			break;
		case SORT_OCTETS:
			qsort(paf, aif.aif_nflows, sizeof(*paf), sort_octets);
			break;
		}

		term_setxy(0, 2);
		for (row = 0, ch = (page - 1) * nrows; row < nrows;
		    row++, ch++) {
			char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
			char last[21];

			term_setxy(0, row + 2);
			if (ch >= aif.aif_nflows) {
				term_clreol();
				continue;
			}
			(void)inet_ntop(AF_INET, &paf[ch]->af_src, src,
			    sizeof(src));
			(void)inet_ntop(AF_INET, &paf[ch]->af_dst, dst,
			    sizeof(dst));
			(void)strftime(last, sizeof(last), "%d-%h-%Y %H:%M:%S",
			    localtime(&paf[ch]->af_last));
			term_printf("%s  %-15s  %-15s  %11u  %11u", last,
			    src, dst, paf[ch]->af_pkts,
			    paf[ch]->af_octets);
		}
		if (opkts > pkts || ooctets > octets)
			opkts = ooctets = 0;
		if (opkts == 0)
			opkts = pkts;
		if (ooctets == 0)
			ooctets = octets;
		term_setxy(0, nrows + 3);
		if (error == 0)
			term_printf(
			    "Page: %d/%d  Pkts/s: %llu  Octets/s: %llu ",
			    page, npages, (pkts - opkts) / seconds,
			    (octets - ooctets) / seconds);
		else
			term_printf("%s ", strerror(error));
		term_clreol();
		opkts = pkts;
		ooctets = octets;
		term_flush();

		switch (term_getch()) {
		case 'U':
			if (page > 1)
				--page;
			continue;
		case 'D':
			if (page < npages)
				page++;
			continue;
		case 'H':
			page = 1;
			continue;
		case 'E':
			page = npages;
			continue;
		case 'R':
			term_setxy(0, 0);
			term_redraw();
			goto redraw;
		}
		if (restart) {
			restart = 0;
			goto rstart;
		}

		aif.aif_nflows = ACCTFLOWS;
		if (ioctl(s, SIOCGFLOWS, &ifr) < 0) {
			error = errno;
			aif.aif_nflows = 0;
		} else
			error = 0;
		npages = aif.aif_nflows / nrows + (aif.aif_nflows % nrows != 0);
	}

	/* NOTREACHED */
	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-r] [-i interface] [-o sortmode] [-s seconds]\n",
	    __progname);
	exit(EX_USAGE);
}

static void
sighandler(int signo)
{
	term_close();
	switch (signo) {
	case SIGWINCH:
		restart++;
		break;
	default:
		_exit(EX_OK);
		/* NOTREACHED */
	}
}

static int
intvalue(const char *s)
{
	long lval;
	char *ep;

	errno = 0;
	lval = strtol(s, &ep, 10);
	if (*s == '\0' || *ep != '\0' || lval <= 0 || lval > INT_MAX) {
		errno = EINVAL;
		return (-1);
	}
	return (lval);
}

static int
sortmode(const char *s)
{
	static char *modes[] =
	    { "none", "last", "src", "dst", "pkts", "octets" };
	unsigned int mode;

	for (mode = 0; mode < sizeof(modes) / sizeof(*modes); mode++)
		if (strcasecmp(modes[mode], s) == 0)
			return (mode);
	errno = EINVAL;
	return (-1);
}


static int
sort_last(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_last < afb->af_last)
		return (-order);
	if (afa->af_last > afb->af_last)
		return (order);
	return (0);
}

static int
sort_src(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_src < afb->af_src)
		return (-order);
	if (afa->af_src > afb->af_src)
		return (order);
	return (0);
}

static int
sort_dst(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_dst < afb->af_dst)
		return (-order);
	if (afa->af_dst > afb->af_dst)
		return (order);
	return (0);
}

static int
sort_pkts(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_pkts < afb->af_pkts)
		return (-order);
	if (afa->af_pkts > afb->af_pkts)
		return (order);
	return (0);
}

static int
sort_octets(const void *a, const void *b)
{
	const struct acct_flow *afa = *(struct acct_flow **)a;
	const struct acct_flow *afb = *(struct acct_flow **)b;

	if (afa->af_octets < afb->af_octets)
		return (-order);
	if (afa->af_octets > afb->af_octets)
		return (order);
	return (0);
}
