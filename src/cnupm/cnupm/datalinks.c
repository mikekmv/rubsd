/*	$RuOBSD: datalinks.c,v 1.10 2005/06/30 16:53:52 form Exp $	*/

/*
 * Copyright (c) 2003-2004 Oleg Safiullin <form@pdp-11.org.ru>
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
#ifdef HAVE_PFLOG
#include <net/if.h>
#include <net/if_pflog.h>
#endif
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <pcap.h>

#include "cnupm.h"
#include "inet6.h"
#include "datalinks.h"
#include "collect.h"

#define ETHER_HDRLEN	14
#define ETHERTYPE_IP	0x0800
#define ETHERTYPE_IPV6	0x86dd
#define ETHER_TYPE(p)	(((u_int16_t *)(p))[6])
#define SLIP_HDRLEN	16
#define PPP_HDRLEN	4
#define PPP_IP		0x21
#define PPP_PROTOCOL(p)	((((u_char *)(p))[2] << 8) + ((u_char *)(p))[3])
#define PPPOE_HDRLEN	6
#define ENC_HDRLEN	12

#define EXTRACT_16BITS(p) \
			((u_int16_t)*((const u_int8_t *)(p) + 0) << 8 | \
			(u_int16_t)*((const u_int8_t *)(p) + 1))

#ifdef DLT_LINUX_SLL
#ifndef SLL_HDRLEN
#define SLL_HDRLEN	16
#endif
#endif

struct datalink_handler {
	int		dh_type;
	pcap_handler	dh_handler;
};

static void dl_null(u_char *, const struct pcap_pkthdr *h, const u_char *);
#ifdef DLT_LOOP
static void dl_loop(u_char *, const struct pcap_pkthdr *h, const u_char *);
#endif
static void dl_ether(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_ppp(u_char *, const struct pcap_pkthdr *h, const u_char *);
#ifdef DLT_PPP_ETHER
static void dl_pppoe(u_char *, const struct pcap_pkthdr *h, const u_char *);
#endif
static void dl_slip(u_char *, const struct pcap_pkthdr *h, const u_char *);
static void dl_raw(u_char *, const struct pcap_pkthdr *h, const u_char *);
#ifdef DLT_LINUX_SLL
static void dl_sll(u_char *, const struct pcap_pkthdr *h, const u_char *);
#endif
#if defined(HAVE_PFLOG) && defined(DLT_PFLOG)
static void dl_pflog(u_char *, const struct pcap_pkthdr *h, const u_char *);
#endif
#ifdef DLT_ENC
static void dl_enc(u_char *, const struct pcap_pkthdr *h, const u_char *);
#endif

static struct datalink_handler datalink_handlers[] = {
	{ DLT_NULL,		dl_null		},
#ifdef DLT_LOOP
	{ DLT_LOOP,		dl_loop		},
#endif
	{ DLT_EN10MB,		dl_ether	},
	{ DLT_IEEE802,		dl_ether	},
	{ DLT_PPP,		dl_ppp		},
#ifdef DLT_PPP_ETHER
	{ DLT_PPP_ETHER,	dl_pppoe	},
#endif
	{ DLT_SLIP,		dl_slip		},
	{ DLT_SLIP_BSDOS,	dl_slip		},
	{ DLT_RAW,		dl_raw		},
#ifdef DLT_LINUX_SLL
	{ DLT_LINUX_SLL,	dl_sll		},
#endif
#if defined(HAVE_PFLOG) && defined(DLT_PFLOG)
	{ DLT_PFLOG,		dl_pflog	},
#endif
#ifdef DLT_ENC
	{ DLT_ENC,		dl_enc		},
#endif
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

#ifdef DLT_LOOP
static void
dl_loop(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	collect((sa_family_t)htonl(*(u_int32_t *)p), p + sizeof(u_int32_t));
}
#endif

static void
dl_ether(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	switch (ntohs(ETHER_TYPE(p))) {
	case ETHERTYPE_IP:
		collect(AF_INET, p + ETHER_HDRLEN);
		break;
	case ETHERTYPE_IPV6:
		collect(AF_INET6, p + ETHER_HDRLEN);
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

#ifdef DLT_PPP_ETHER
static void
dl_pppoe(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	p += PPPOE_HDRLEN;
	if (EXTRACT_16BITS(p) == PPP_IP)
		collect(AF_INET, p + sizeof(u_int16_t));
}
#endif	/* DLT_PPP_ETHER */

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

#ifdef DLT_LINUX_SLL
static void
dl_sll(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct ip *ip = (struct ip *)(p + SLL_HDRLEN);

	switch (ip->ip_v) {
	case 4:
		collect(AF_INET, ip);
		break;
	case 6:
		collect(AF_INET6, ip);
		break;
	}
}
#endif	/* DLT_LINUX_SLL */

#if defined(HAVE_PFLOG) && defined(DLT_PFLOG)
static void
dl_pflog(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct pfloghdr *pfh = (struct pfloghdr *)p;

	switch (pfh->af) {
	case AF_INET:
	case AF_INET6:
		collect(pfh->af, (struct ip *)(p + BPF_WORDALIGN(pfh->length)));
		break;
	}
}
#endif	/* HAVE_PFLOG && DLT_PFLOG */

#ifdef DLT_ENC
static void
dl_enc(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	collect(AF_INET, p + ENC_HDRLEN);
}
#endif	/* DLT_ENC */
