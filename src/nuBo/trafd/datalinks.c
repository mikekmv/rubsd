/*	$RuOBSD: datalinks.c,v 1.3 2003/05/16 13:17:28 form Exp $	*/

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

#include <sys/types.h>
#include <sys/socket.h>
#include <machine/endian.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ppp_defs.h>
#include <net/slip.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <pcap.h>
#include <syslog.h>

#include "datalinks.h"
#include "collect.h"

char *device = NULL;

struct handler {
	int		type;
	pcap_handler	handler;
};

static void handle_null(u_char *, const struct pcap_pkthdr *, const u_char *);
static void handle_loop(u_char *, const struct pcap_pkthdr *, const u_char *);
static void handle_ether(u_char *, const struct pcap_pkthdr *, const u_char *);
static void handle_ppp(u_char *, const struct pcap_pkthdr *, const u_char *);
static void handle_slip(u_char *, const struct pcap_pkthdr *, const u_char *);
static void collect(int, const void *);

static struct handler handlers[] = {
	{ DLT_NULL,		handle_null },
	{ DLT_LOOP,		handle_loop },
	{ DLT_EN10MB,		handle_ether },
	{ DLT_IEEE802,		handle_ether },
	{ DLT_PPP,		handle_ppp },
	{ DLT_SLIP,		handle_slip },
	{ DLT_SLIP_BSDOS,	handle_slip },
	{ -1,			NULL }
};

pcap_handler
lookup_datalink_handler(int type)
{
	struct handler *p;

	for (p = handlers; p->type >= 0; p++)
		if (p->type == type)
			return (p->handler);
	return (NULL);
}

static void
handle_null(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	sa_family_t family = (sa_family_t)*(u_int32_t *)p;

	collect(family, p + sizeof(u_int32_t));
}

static void
handle_loop(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	sa_family_t family = (sa_family_t)htonl(*(u_int32_t *)p);

	collect(family, p + sizeof(u_int32_t));
}

static void
handle_ether(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct ether_header *ep = (struct ether_header *)p;

	p += sizeof(struct ether_header);
	switch (ntohs(ep->ether_type)) {
	case ETHERTYPE_IP:
		collect(AF_INET, p);
		break;
#ifdef INET6
	case ETHERTYPE_IPV6:
		collect(AF_INET6, p);
		break;
#endif
	}
}

static void
handle_ppp(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	switch (PPP_PROTOCOL(p)) {
	case PPP_IP:
	case ETHERTYPE_IP:
		collect(AF_INET, p + PPP_HDRLEN);
		break;
	}
}

static void
handle_slip(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct ip *ip = (struct ip *)(p + SLIP_HDRLEN);
	switch (ip->ip_v) {
	case 4:
		collect(AF_INET, ip);
		break;
#ifdef INET6
	case 6:
		collect(AF_INET6, ip);
		break;
#endif
	}
}

static void
collect(int family, const void *pkt)
{
	if (ic_collect(family, pkt) > 0 && ic_dump(device) < 0)
		syslog(LOG_ERR, "ic_dump: %m");
}
