/*	$RuOBSD: datalinks.c,v 1.2 2004/01/14 05:26:50 form Exp $	*/

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

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ppp_defs.h>
#include <net/slip.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <pcap.h>

#include "datalinks.h"
#include "collect.h"

struct datalink_handler {
	int		dh_type;
	pcap_handler	dh_handler;
};

static void dl_null(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_loop(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_ether(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_ppp(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_slip(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_raw(u_char *, const struct pcap_pkthdr *h, const u_char *);

static struct datalink_handler datalink_handlers[] = {
	{ DLT_NULL,		dl_null		},
	{ DLT_LOOP,		dl_loop		},
	{ DLT_EN10MB,		dl_ether	},
	{ DLT_IEEE802,		dl_ether	},
	{ DLT_PPP,		dl_ppp		},
	{ DLT_SLIP,		dl_slip		},
	{ DLT_SLIP_BSDOS,	dl_slip		},
	{ DLT_RAW,		dl_raw		},
	{ -1,			NULL		}
};

pcap_handler
lookup_datalink_handler(int type)
{
	struct datalink_handler *dh;

	for (dh = datalink_handlers; dh->dh_type >= 0; dh++)
		if (dh->dh_type == type)
			return (dh->dh_handler);
	return (NULL);
}

static void
dl_null(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	collect((sa_family_t)*(u_int32_t *)p, p + sizeof(u_int32_t));
}

static void
dl_loop(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	collect((sa_family_t)htonl(*(u_int32_t *)p), p + sizeof(u_int32_t));
}

static void
dl_ether(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct ether_header *ep = (struct ether_header *)p;

	p += sizeof(struct ether_header);
	switch (ntohs(ep->ether_type)) {
	case ETHERTYPE_IP:
		collect(AF_INET, p);
		break;
	case ETHERTYPE_IPV6:
		collect(AF_INET6, p);
		break;
	}
}

static void
dl_ppp(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	switch (PPP_PROTOCOL(p)) {
	case PPP_IP:
	case ETHERTYPE_IP:
		collect(AF_INET, p + PPP_HDRLEN);
		break;
	}
}

static void
dl_slip(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct ip *ip = (struct ip *)(p + SLIP_HDRLEN);
	switch (ip->ip_v) {
	case 4:
		collect(AF_INET, ip);
		break;
	case 6:
		collect(AF_INET6, ip);
		break;
	}
}

static void
dl_raw(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	collect(AF_INET, p);
}
