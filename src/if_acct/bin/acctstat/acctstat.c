/*	$RuOBSD: acctstat.c,v 1.1.1.1 2004/10/27 06:32:39 form Exp $	*/

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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_acct.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>


int main(int, char **);
__dead static void usage(void);


static char *interface = "acct0";
static int reset;


int
main(int argc, char **argv)
{
	struct acct_flow af[ACCTFLOWS];
	struct acctio_flows aif = { af, ACCTFLOWS };
	struct ifreq ifr;
	int ch, s;

	while ((ch = getopt(argc, argv, "i:r")) != -1)
		switch (ch) {
		case 'i':
			interface = optarg;
			break;
		case 'r':
			reset++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	if (argc - optind != 0)
		usage();

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err(EX_UNAVAILABLE, "socket");

	bzero(&ifr, sizeof(ifr));
	(void)strlcpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	ifr.ifr_data = (caddr_t)&aif;

	if (ioctl(s, reset ? SIOCGRFLOWS : SIOCGFLOWS, &ifr) < 0)
		err(EX_UNAVAILABLE, "ioctl");

	for (ch = 0; ch < aif.aif_nflows; ch++) {
		char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
		char first[17], last[17];

		(void)inet_ntop(AF_INET, &aif.aif_flows[ch].af_src, src,
		    sizeof(src));
		(void)inet_ntop(AF_INET, &aif.aif_flows[ch].af_dst, dst,
		    sizeof(dst));
		(void)strftime(first, sizeof(first), "%Y-%m-%d %H:%M",
		    localtime(&aif.aif_flows[ch].af_first));
		(void)strftime(last, sizeof(last), "%Y-%m-%d %H:%M",
		    localtime(&aif.aif_flows[ch].af_last));

		printf("%s %s %15s %15s %u\n", first, last, src, dst,
		    aif.aif_flows[ch].af_octets);
	}

	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-r] [-i interface]\n", __progname);
	exit(EX_USAGE);
}
