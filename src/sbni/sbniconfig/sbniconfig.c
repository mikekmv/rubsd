/*	$RuOBSD$	*/

/*
 * Copyright (c) 2001 Oleg Safiullin
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
#include <sys/sockio.h>
#include <sys/ioctl.h>

#include <net/if.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include <sbnivar.h>

extern char *__progname;

static char *bauds[] = { "62.5", "125", "250", "500", "1000", "2000" };

int main __P((int, char **));
void usage __P((void));
void sbni_print_stats __P((int, struct ifreq *));
void sbni_set_flags __P((int, struct ifreq *, char *));
void sbni_setup_link __P((int, struct ifreq *, char *, char *));

int
main(argc, argv)
	int argc;
	char **argv;
{
	int s;
	struct ifreq ifr;

	if (argc < 2 || argc > 4)
		usage();

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err(1, "socket");

	(void) strlcpy(ifr.ifr_name, argv[1], sizeof (ifr.ifr_name));

	switch (argc) {
	case 2:
		sbni_print_stats(s, &ifr);
		break;
	case 3:
		sbni_set_flags(s, &ifr, argv[2]);
		break;
	case 4:
		sbni_setup_link(s, &ifr, argv[2], argv[3]);
		break;
	}

	return (0);
}

void
usage(void)
{
	fprintf(stderr,
	    "usage: %s interface [baud rxl]\n"
	    "       %s interface flags\n",
	    __progname, __progname);
	exit(1);
}

void
sbni_print_stats(s, ifr)
	int s;
	struct ifreq *ifr;
{
	u_int32_t flags;
	struct sbni_flags *sf = (void *)&flags;
	u_int32_t info;
	struct sbni_info *si = (void *)&info;
	u_int baud;

	if (ioctl(s, SIOCGHWFLAGS, ifr) < 0)
		err(1, "SIOCGHWFLAGS");
	flags = *(u_int32_t *)&ifr->ifr_data;

	if (ioctl(s, SIOCGIFINFO, ifr) < 0)
		err(1, "SIOCGIFINFO");
	info = *(u_int32_t *)&ifr->ifr_data;
	baud = 62500 * si->si_baud;

	printf("%s:\tMAC address: 00:ff:01:%02x:%02x:%02x\n",
	    ifr->ifr_name,
#if BYTE_ORDER == BIG_ENDIAN
	    sf->sf_enaddr[0],
	    sf->sf_enaddr[1],
	    sf->sf_enaddr[2]);
#else
	    sf->sf_enaddr[2],
	    sf->sf_enaddr[1],
	    sf->sf_enaddr[0]);
#endif
	printf("\tBaud rate: %d%s kbit/s (%s speed mode), receive level %d\n",
	    baud / 1000, baud % 1000 ? ".5" : "",
	    si->si_lo_speed ? "low" : "high",
	    si->si_rxl);
}

void
sbni_set_flags(s, ifr, flags)
	int s;
	struct ifreq *ifr;
	char *flags;
{
	u_int32_t sf;
	char *ep;

	if (((sf = strtoul(flags, &ep, 0)) == ULONG_MAX && errno == ERANGE)
	    || *ep != 0)
		errx(1, "bad flags value");

	ifr->ifr_data = *(caddr_t*)&sf;
	if (ioctl(s, SIOCSHWFLAGS, ifr) < 0)
		err(1, "SIOCSHWFLAGS");
}

void
sbni_setup_link(s, ifr, baud, rxl)
	int s;
	struct ifreq *ifr;
	char *baud, *rxl;
{
	u_int32_t flags = 0;
	struct sbni_flags *sf = (void *)&flags;
	u_int32_t info;
	struct sbni_info *si = (void *)&info;
	char *ep;
	int i;

	if (ioctl(s, SIOCGIFINFO, ifr) < 0)
		err(1, "SIOCGIFINFO");
	info = *(u_int32_t *)&ifr->ifr_data;

	if (!strcmp(baud, "auto"))
		i = -1;
	else {
		for (i = 0; i < sizeof(bauds); i++)
			if (!strcmp(baud, bauds[i]))
				break;
	}
	if (i >= sizeof(bauds))
		errx(1, "invalid baud rate");

	if (i >= 0) {
		if ((si->si_lo_speed && i > 3) || (!si->si_lo_speed && i < 2))
			errx(1, "can't set baud to %s kbit/s in %s speed mode",
			    bauds[i], si->si_lo_speed ? "low" : "high");
		sf->sf_fx_baud = 1;
		sf->sf_baud = si->si_lo_speed ? 3 - i : 5 - i;
	}

	if (strcmp(rxl, "auto")) {
		u_long val = strtoul(rxl, &ep, 0);

		if (*ep != 0 || val > 15)
			errx(1, "invalid receive level");
		sf->sf_fx_rxl = 1;
		sf->sf_rxl = val;
	}

	ifr->ifr_data = *(caddr_t*)&flags;
	if (ioctl(s, SIOCSHWFLAGS, ifr) < 0)
		err(1, "SIOCSHWFLAGS");
}
