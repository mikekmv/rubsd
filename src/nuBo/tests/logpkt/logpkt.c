/*	$RuOBSD$	*/

/*
 * Copyright (c) 2003 Oleg Safiullin
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
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "printers.h"

static int snaplen = DEF_SNAPLEN;
static char *device;
static int Oflag;
static int pflag;
pcap_t *pd;

int main(int, char **);
__dead static void usage(void);
static char *copy_argv(char **);
static void cleanup(int signo);

int
main(int argc, char **argv)
{
	char *filter, ebuf[PCAP_ERRBUF_SIZE];
	struct bpf_program bprog;
	pcap_handler printer;
	int ch;

	while ((ch = getopt(argc, argv, "i:Ops:")) != -1)
		switch (ch) {
		case 'i':
			device = optarg;
			break;
		case 'O':
			Oflag = 1;
			break;
		case 'p':
			pflag = 1;
			break;
		case 's':
			{
				char *ep;
				u_long l;

				errno = 0;
				l = strtoul(optarg, &ep, 0);
				if (*ep != '\0' || errno == ERANGE ||
				    l > INT_MAX)
					errx(1, "Invalid snaplen");
				snaplen = (int)l;
			}
			break;
		default:
			usage();
			/* NOTREACHED */
		}

	if (device == NULL && (device = pcap_lookupdev(ebuf)) == NULL)
		errx(1, "%s", ebuf);
	if ((pd = pcap_open_live(device, snaplen, !pflag, 1000, ebuf)) == NULL)
		errx(1, "%s", ebuf);

	(void)seteuid(getuid());
	(void)setuid(getuid());

	filter = copy_argv(argv + optind);
	if (pcap_compile(pd, &bprog, filter, Oflag, 0) < 0 ||
	    pcap_setfilter(pd, &bprog) < 0)
		errx(1, "%s", pcap_geterr(pd));
	pcap_freecode(&bprog);
	if (filter != NULL)
		free(filter);

	ch = pcap_datalink(pd);
	if ((printer = lookup_printer(ch)) == NULL)
		errx(1, "Unknown datalink type %d for interface %s",
		    ch, device);

	(void)printf("Listening on interface %s\n", device);

	(void)signal(SIGTERM, cleanup);
	(void)signal(SIGINT, cleanup);
	(void)signal(SIGHUP, cleanup);

	if (pcap_loop(pd, -1, printer, NULL) < 0)
		errx(1, "pcap_loop: %s", pcap_geterr(pd));
	pcap_close(pd);

	return (0);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-Op] [-i interface] [-s snaplen] [expression]\n",
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
cleanup(int signo)
{
	struct pcap_stat ps;

	printf("\n");
	(void)fflush(stdout);
	if (pcap_stats(pd, &ps) < 0)
		warnx("pcap_stats: %s", pcap_geterr(pd));
	else
		printf("%d packets received by filter\n"
		    "%d packets dropped by kernel\n", ps.ps_recv, ps.ps_drop);
	if (signo != SIGHUP)
		_exit(1);
	printf("\n");
	(void)fflush(stdout);
}
