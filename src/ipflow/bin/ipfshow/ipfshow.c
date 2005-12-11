/*	$RuOBSD: ipfshow.c,v 1.3 2005/10/28 17:06:04 form Exp $	*/

/*
 * Copyright (c) 2005 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <net/if.h>
#include <net/ipflow.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "flow.h"
#include "term.h"


static int human;
static int seconds = 1;
static int smode = -1;
static int order = FLOW_ORDER_ASCENDING;
static int restart;


int main(int, char **);
__dead static void usage(void);
static void sighandler(int);
static int intvalue(const char *);
static char *format_octets(u_int64_t);


int
main(int argc, char **argv)
{
	char head[60];
	struct ipflow *ifl;
	struct ipflow_req irq;
	struct ipflow **pifl;
	struct ipflow_version iv;
	struct sigaction sa;
	struct utsname un;
	u_int64_t pkts, octets, opkts = 0, ooctets = 0;
	int ch, fd, page, npages, row, nrows, error = 0;
	size_t maxflows;

	while ((ch = getopt(argc, argv, "ho:rs:")) != -1)
		switch (ch) {
		case 'h':
			human++;
			break;
		case 'o':
			if ((smode = flow_sort_mode(optarg)) < 0)
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
		smode = FLOW_SORT_LAST;
		order = -order;
	}
	flow_sort_setorder(order);

	if ((ifl = malloc(sizeof(*ifl) * IPFLOW_MAX_FLOWS)) == NULL ||
	    (pifl = malloc(sizeof(*pifl) * IPFLOW_MAX_FLOWS)) == NULL)
		err(EX_UNAVAILABLE, "malloc");

	irq.irq_flows = ifl;
	irq.irq_nflows = IPFLOW_MAX_FLOWS;

	if ((fd = open(_PATH_DEV_IPFLOW, O_RDONLY)) < 0)
		err(EX_UNAVAILABLE, "open: %s", _PATH_DEV_IPFLOW);

	if (ioctl(fd, IIOCVERSION, &iv) < 0)
		err(EX_IOERR, "IIOCVERSION");

	if (iv.iv_major != IPFLOW_VERSION_MAJOR)
		errx(EX_CONFIG, "Unsupported ipflow version %u.%u",
		    iv.iv_major, iv.iv_minor);

	if (ioctl(fd, IIOCGNFLOWS, &maxflows) < 0)
		err(EX_IOERR, "IIOCGNFLOWS");

	if ((irq.irq_nflows = read(fd, irq.irq_flows,
	    IPFLOW_MAX_FLOWS * sizeof(struct ipflow))) < 0)
		err(EX_IOERR, "read: %s", _PATH_DEV_IPFLOW);
	else
		irq.irq_nflows /= sizeof(struct ipflow);

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
	(void)snprintf(head, sizeof(head), "%s/%s %s (%s)",
	    un.sysname, un.machine, un.release, un.nodename);
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
	npages = irq.irq_nflows / nrows + (irq.irq_nflows % nrows != 0);
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

		for (ch = pkts = octets = 0; ch < irq.irq_nflows; ch++) {
			pifl[ch] = &ifl[ch];
			pkts += irq.irq_flows[ch].if_pkts;
			octets += irq.irq_flows[ch].if_octets;
		}
		flow_sort(pifl, irq.irq_nflows, smode);

		term_setxy(0, 2);
		for (row = 0, ch = (page - 1) * nrows; row < nrows;
		    row++, ch++) {
			char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
			char last[21], pkt[12];

			term_setxy(0, row + 2);
			if (ch >= irq.irq_nflows) {
				term_clreol();
				continue;
			}
			(void)inet_ntop(AF_INET, &pifl[ch]->if_src, src,
			    sizeof(src));
			(void)inet_ntop(AF_INET, &pifl[ch]->if_dst, dst,
			    sizeof(dst));
			(void)strftime(last, sizeof(last), "%d-%h-%Y %H:%M:%S",
			    localtime(&pifl[ch]->if_last));
			if (pifl[ch]->if_pkts < 100000000000LLU)
				(void)snprintf(pkt, sizeof(pkt), "%11llu",
				    pifl[ch]->if_pkts);
			else
				(void)snprintf(pkt, sizeof(pkt), "***********");
			term_printf("%s  %-15s  %-15s  %s  %11s", last,
			    src, dst, pkt, format_octets(pifl[ch]->if_octets));
		}
		if (opkts > pkts || ooctets > octets)
			opkts = ooctets = 0;
		if (opkts == 0)
			opkts = pkts;
		if (ooctets == 0)
			ooctets = octets;
		term_setxy(0, nrows + 3);
		if (error == 0)
			term_printf("Page: %d/%d  Flows: %d/%d  "
			    "Pkts/s: %llu  Octets/s: %llu ",
			    page, npages, irq.irq_nflows, maxflows,
			    (pkts - opkts) / seconds,
			    (octets - ooctets) / seconds);
		else
			term_printf("%s ", strerror(error));
		term_clreol();
		opkts = pkts;
		ooctets = octets;
		term_flush();

		switch (term_getch()) {
		case TERM_KB_UP:
			if (page > 1)
				--page;
			continue;
		case TERM_KB_DOWN:
			if (page < npages)
				page++;
			continue;
		case TERM_KB_HOME:
			page = 1;
			continue;
		case TERM_KB_END:
			page = npages;
			continue;
		case TERM_KB_REFRESH:
			term_setxy(0, 0);
			term_redraw();
			goto redraw;
		case 'Q':
			raise(SIGINT);
			continue;
		case 'R':
			order = -order;
			flow_sort_setorder(order);
			continue;
		case 'H':
			human = !human;
			break;
		case 'F':
			smode = FLOW_SORT_FIRST;
			continue;
		case 'L':
			smode = FLOW_SORT_LAST;
			continue;
		case 'S':
			smode = FLOW_SORT_SRC;
			continue;
		case 'D':
			smode = FLOW_SORT_DST;
			continue;
		case 'P':
			smode = FLOW_SORT_PKTS;
			continue;
		case 'O':
			smode = FLOW_SORT_OCTETS;
			continue;
		}
		if (restart) {
			restart = 0;
			goto rstart;
		}

		if ((irq.irq_nflows = read(fd, irq.irq_flows,
		    IPFLOW_MAX_FLOWS * sizeof(struct ipflow))) < 0 ||
		    ioctl(fd, IIOCGNFLOWS, &maxflows) < 0) {
			error = errno;
			irq.irq_nflows = 0;
			maxflows = 0;
		} else {
			irq.irq_nflows /= sizeof(struct ipflow);
			error = 0;
		}
		npages = irq.irq_nflows / nrows + (irq.irq_nflows % nrows != 0);
	}

	/* NOTREACHED */
	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-hr] [-o sortmode] [-s seconds]\n", __progname);
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

static char *
format_octets(u_int64_t octets)
{
	static char oct[12];

	if (human) {
		if (octets >= 11152921504606846976LLU)
			(void)snprintf(oct, sizeof(oct), "%.3fE",
			    (double)octets / 11152921504606846976.);
		else if (octets >= 1125899906842624LLU)
			(void)snprintf(oct, sizeof(oct), "%.3fP",
			    (double)octets / 1125899906842624.);
		else if (octets >= 1099511627776LLU)
			(void)snprintf(oct, sizeof(oct), "%.3fT",
			    (double)octets / 1099511627776.);
		else if (octets >= 1073741824LLU)
			(void)snprintf(oct, sizeof(oct), "%.3fG",
			    (double)octets / 1073741824.);
		else if (octets >= 1048576LLU)
			(void)snprintf(oct, sizeof(oct), "%.3fM",
			    (double)octets / 1048576.);
		else
			(void)snprintf(oct, sizeof(oct), "%.3fK",
			    (double)octets / 1024.);
	} else {
		if (octets < 100000000000LLU)
			(void)snprintf(oct, sizeof(oct), "%11llu", octets);
		else
			(void)snprintf(oct, sizeof(oct), "***********");
	}

	return (oct);
}
