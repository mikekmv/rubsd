/*	$RuOBSD: ipflow_collect.c,v 1.5 2005/11/02 16:51:46 form Exp $	*/

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
#include <sys/mutex.h>
#include <sys/conf.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/select.h>
#include <sys/tree.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/bpf.h>
#include <net/ppp_defs.h>
#include <net/slip.h>
#include <net/if_pflog.h>
#include <net/if_enc.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include "ipflow.h"


#define NULL_HDRLEN		4
#define PPPOE_HDRLEN		6
#define MIN_PFLOG_HDRLEN	45
#define ETHER_TYPE(p)		(((u_int16_t *)(p))[6])
#define EXTRACT_16BITS(p)	\
	((u_int16_t)*((const u_int8_t *)(p) + 0) << 8 | \
	(u_int16_t)*((const u_int8_t *)(p) + 1))


static void ipflow_collect_null(void *, int);
static void ipflow_collect_loop(void *, int);
static void ipflow_collect_ether(void *, int);
static void ipflow_collect_ppp(void *, int);
static void ipflow_collect_pppoe(void *, int);
static void ipflow_collect_slip(void *, int);
static void ipflow_collect_pflog(void *, int);
static void ipflow_collect_enc(void *, int);


struct ipflow_collect {
	int			ic_dlt;
	ipflow_collector	ic_collect;
};


struct ipflow_tree ipflow_tree;
struct ipflow_entry *ipflow_entries;

struct mutex ipflow_mtx;

static struct ipflow_collect ipflow_collectors[] = {
	{ DLT_NULL,		ipflow_collect_null	},
	{ DLT_LOOP,		ipflow_collect_loop	},
	{ DLT_EN10MB,		ipflow_collect_ether	},
	{ DLT_IEEE802,		ipflow_collect_ether	},
	{ DLT_PPP,		ipflow_collect_ppp	},
	{ DLT_PPP_ETHER,	ipflow_collect_pppoe	},
	{ DLT_SLIP,		ipflow_collect_slip	},
	{ DLT_PFLOG,		ipflow_collect_pflog	},
	{ DLT_ENC,		ipflow_collect_enc	},
	{ DLT_RAW,		ipflow_collect		},
	{ -1,			NULL			}
};


RB_GENERATE(ipflow_tree, ipflow_entry, ife_entry, ipflow_compare)


int
ipflow_init(void)
{
	mtx_init(&ipflow_mtx, IPL_VM);
	ipflow_flush();
	if ((ipflow_entries = malloc(ipflow_maxflows *
	    sizeof(struct ipflow_entry), M_DEVBUF, M_NOWAIT)) == NULL)
		return (ENOMEM);

	return (0);
}

int
ipflow_realloc(u_int maxflows)
{
	struct ipflow_entry *p;
	vaddr_t va;
	u_int i;

	if (ipflow_maxflows == maxflows)
		return (0);
	if (maxflows < IPFLOW_MIN_FLOWS || maxflows > IPFLOW_MAX_FLOWS)
		return (EINVAL);
	if (maxflows < ipflow_nflows)
		return (EBUSY);

	if ((p = malloc(maxflows *
	    sizeof(struct ipflow_entry), M_DEVBUF, M_NOWAIT)) == NULL)
		return (ENOMEM);

	mtx_enter(&ipflow_mtx);
	if (ipflow_nflows != 0) {
		bcopy(ipflow_entries, p,
		    ipflow_nflows * sizeof(struct ipflow_entry));
		va = (caddr_t)ipflow_entries - (caddr_t)p;
		(caddr_t)RB_ROOT(&ipflow_tree) -= va;
		for (i = 0; i < ipflow_nflows; i++) {
			if (RB_PARENT(&p[i], ife_entry) != NULL)
				(caddr_t)RB_PARENT(&p[i], ife_entry) -= va;
			if (RB_LEFT(&p[i], ife_entry) != NULL)
				(caddr_t)RB_LEFT(&p[i], ife_entry) -= va;
			if (RB_RIGHT(&p[i], ife_entry) != NULL)
				(caddr_t)RB_RIGHT(&p[i], ife_entry) -= va;
		}
	}
	free(ipflow_entries, M_DEVBUF);
	ipflow_entries = p;
	ipflow_maxflows = maxflows;
	mtx_leave(&ipflow_mtx);

	return (0);
}

void
ipflow_free(void)
{
	ipflow_flush();
	if (ipflow_entries != NULL) {
		free(ipflow_entries, M_DEVBUF);
		ipflow_entries = NULL;
	}
}

void
ipflow_flush(void)
{
	mtx_enter(&ipflow_mtx);
	ipflow_nflows = ipflow_dropped = 0;
	RB_INIT(&ipflow_tree);
	mtx_leave(&ipflow_mtx);
}

void
ipflow_collect(void *data, int caplen)
{
	struct ip *ip = data;
	struct ipflow ifl;

	ifl.if_first = ifl.if_last = time.tv_sec;
	ifl.if_src = ip->ip_src.s_addr;
	ifl.if_dst = ip->ip_dst.s_addr;
	ifl.if_pkts = 1;
	ifl.if_octets = htons(ip->ip_len);
	ipflow_insert(&ifl);
}

void
ipflow_insert(struct ipflow *ifl)
{
	struct ipflow_entry *ife;

	mtx_enter(&ipflow_mtx);
	if ((ife = RB_FIND(ipflow_tree, &ipflow_tree,
	    (struct ipflow_entry *)ifl)) == NULL) {
		if (ipflow_nflows >= ipflow_maxflows) {
			ipflow_dropped++;
			return;
		}
		ife = &ipflow_entries[ipflow_nflows++];
		ife->ife_first = ifl->if_first;
		ife->ife_last = ifl->if_last;
		ife->ife_src = ifl->if_src;
		ife->ife_dst = ifl->if_dst;
		ife->ife_pkts = ifl->if_pkts;
		ife->ife_octets = ifl->if_octets;
		(void)RB_INSERT(ipflow_tree, &ipflow_tree, ife);
		if (ipflow_nflows == 1) {
			selwakeup(&ipflow_rsel);
			KNOTE(&ipflow_rsel.si_note, 0);
		}
	} else {
		ife->ife_pkts += ifl->if_pkts;
		ife->ife_octets += ifl->if_octets;
		if (ife->ife_first > ifl->if_first)
			ife->ife_first = ifl->if_first;
		if (ife->ife_last < ifl->if_last)
			ife->ife_last = ifl->if_last;
	}
	mtx_leave(&ipflow_mtx);
}

ipflow_collector
ipflow_lookup_collector(int dlt)
{
	int i;

	for (i = 0; ipflow_collectors[i].ic_dlt != -1; i++)
		if (ipflow_collectors[i].ic_dlt == dlt)
			return (ipflow_collectors[i].ic_collect);

	return (NULL);
}

static void
ipflow_collect_null(void *data, int caplen)
{
	if (caplen < NULL_HDRLEN + sizeof(struct ip))
		return;

	switch (*(u_int32_t *)data) {
	case AF_INET:
		ipflow_collect((caddr_t)data + NULL_HDRLEN,
		    caplen - NULL_HDRLEN);
		break;
	}
}

static void
ipflow_collect_loop(void *data, int caplen)
{
	if (caplen < NULL_HDRLEN + sizeof(struct ip))
		return;

	*(u_int32_t *)data = ntohl(*(u_int32_t *)data);
	ipflow_collect_null(data, caplen);
}

static void
ipflow_collect_ether(void *data, int caplen)
{
	if (caplen < ETHER_HDR_LEN + sizeof(struct ip))
		return;

	switch (ntohs(ETHER_TYPE(data))) {
	case ETHERTYPE_IP:
		ipflow_collect((caddr_t)data + ETHER_HDR_LEN,
		    caplen - ETHER_HDR_LEN);
		break;
	}
}

static void
ipflow_collect_ppp(void *data, int caplen)
{
	if (caplen < PPP_HDRLEN + sizeof(struct ip))
		return;

	switch (PPP_PROTOCOL(data)) {
	case PPP_IP:
	case ETHERTYPE_IP:
		ipflow_collect((caddr_t)data + PPP_HDRLEN,
		    caplen - PPP_HDRLEN);
		break;
	}
}

static void
ipflow_collect_pppoe(void *data, int caplen)
{
	if (caplen < PPPOE_HDRLEN + sizeof(struct ip) + sizeof(u_int16_t))
		return;

	switch (EXTRACT_16BITS((caddr_t)data + PPPOE_HDRLEN)) {
	case PPP_IP:
		ipflow_collect((caddr_t)data + PPPOE_HDRLEN + sizeof(u_int16_t),
		    caplen - PPP_HDRLEN);
		break;
	}
}

static void
ipflow_collect_slip(void *data, int caplen)
{
	struct ip *ip = (struct ip *)((caddr_t)data + SLIP_HDRLEN);

	if (caplen < SLIP_HDRLEN + sizeof(struct ip))
		return;

	switch (ip->ip_v) {
	case 4:
		ipflow_collect(ip, caplen - SLIP_HDRLEN);
		break;
	}
}

static void
ipflow_collect_pflog(void *data, int caplen)
{
	struct pfloghdr *hdr = (struct pfloghdr *)data;

	if (caplen < MIN_PFLOG_HDRLEN ||
	    caplen < BPF_WORDALIGN(hdr->length) + sizeof(struct ip))
		return;

	switch (hdr->af) {
	case AF_INET:
		ipflow_collect((caddr_t)data + BPF_WORDALIGN(hdr->length),
		    caplen - BPF_WORDALIGN(hdr->length));
		break;
	}
}

static void
ipflow_collect_enc(void *data, int caplen)
{
	struct enchdr *hdr = data;

	if (caplen < ENC_HDRLEN + sizeof(struct ip))
		return;

	switch (hdr->af) {
	case AF_INET:
		ipflow_collect((caddr_t)data + ENC_HDRLEN,
		    caplen - ENC_HDRLEN);
		break;
	}
}
