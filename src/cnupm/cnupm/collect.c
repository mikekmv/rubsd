/*	$RuOBSD: collect.c,v 1.12 2004/11/07 08:53:08 form Exp $	*/

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

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/tree.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#ifdef HAVE_INET6
#include <netinet/ip6.h>
#endif
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "cnupm.h"
#include "inet6.h"
#include "collect.h"

#define CNUPM_VERSION	(CNUPM_VERSION_MAJOR | (CNUPM_VERSION_MINOR << 8))

#define ENTRIES_TO_SAVE	64
#define DUMP_FILE_MODE	0640

#ifndef IP_OFFMASK
#define IP_OFFMASK	0x1fff
#endif

struct ct_entry {
	RB_ENTRY(ct_entry)	ce_entry;
	struct coll_traffic	ce_traffic;
#define ce_family		ce_traffic.ct_family
#define ce_proto		ce_traffic.ct_proto
#define ce_src			ce_traffic.ct_src
#define ce_dst			ce_traffic.ct_dst
#define ce_sport		ce_traffic.ct_sport
#define ce_dport		ce_traffic.ct_dport
#define ce_bytes		ce_traffic.ct_bytes
};

int ct_entries_max = DEF_CT_ENTRIES;
static int ct_entries_count;
static time_t collect_start;
static struct ct_entry *ct_entries;
u_int32_t collect_lost_packets;
int collect_need_dump;
sa_family_t collect_family = AF_UNSPEC;
int collect_proto = 1;
int collect_ports = 1;

RB_HEAD(ct_tree, ct_entry) ct_head;

static __inline int
ct_entry_compare(struct ct_entry *a, struct ct_entry *b)
{
	int diff;

	if ((diff = a->ce_proto - b->ce_proto) != 0)
		return (diff);
	if ((diff = a->ce_family - b->ce_family) != 0)
		return (diff);
	switch (a->ce_family) {
	case AF_INET:
		diff = a->ce_src.ua_in.s_addr - b->ce_src.ua_in.s_addr;
		if (diff != 0)
			return (diff);
		diff = a->ce_dst.ua_in.s_addr - b->ce_dst.ua_in.s_addr;
		if (diff != 0)
			return (diff);
		break;
	case AF_INET6:
		if ((diff = a->ce_src.ua_in6.s6_addr32[0] -
		    b->ce_src.ua_in6.s6_addr32[0]) != 0)
			return (diff);
		if ((diff = a->ce_src.ua_in6.s6_addr32[1] -
		    b->ce_src.ua_in6.s6_addr32[1]) != 0)
			return (diff);
		if ((diff = a->ce_src.ua_in6.s6_addr32[2] -
		    b->ce_src.ua_in6.s6_addr32[2]) != 0)
			return (diff);
		if ((diff = a->ce_src.ua_in6.s6_addr32[3] -
		    b->ce_src.ua_in6.s6_addr32[3]) != 0)
			return (diff);
		if ((diff = a->ce_dst.ua_in6.s6_addr32[0] -
		    b->ce_dst.ua_in6.s6_addr32[0]) != 0)
			return (diff);
		if ((diff = a->ce_dst.ua_in6.s6_addr32[1] -
		    b->ce_dst.ua_in6.s6_addr32[1]) != 0)
			return (diff);
		if ((diff = a->ce_dst.ua_in6.s6_addr32[2] -
		    b->ce_dst.ua_in6.s6_addr32[2]) != 0)
			return (diff);
		if ((diff = a->ce_dst.ua_in6.s6_addr32[3] -
		    b->ce_dst.ua_in6.s6_addr32[3]) != 0)
			return (diff);
		break;
	default:
		return (0);
	}
	if ((diff = a->ce_sport - b->ce_sport) != 0)
		return (diff);
	if ((diff = a->ce_dport - b->ce_dport) != 0)
		return (diff);
	return (0);
}

RB_PROTOTYPE(ct_tree, ct_entry, ce_entry, ct_entry_compare)

RB_GENERATE(ct_tree, ct_entry, ce_entry, ct_entry_compare)

int
collect_init(int alloc)
{
	ct_entries_count = collect_need_dump = 0;
	collect_start = time(NULL);
	RB_INIT(&ct_head);
	if (alloc) {
		collect_lost_packets = 0;
		ct_entries = calloc(ct_entries_max, sizeof(struct ct_entry));
		if (ct_entries == NULL)
			return (-1);
	}

	return (0);
}

void
collect(sa_family_t family, const void *p)
{
	struct ip *ip = (struct ip *)p;
	struct ip6_hdr *ip6 = (struct ip6_hdr *)p;
	struct ct_entry *ce;

	if (collect_family != AF_UNSPEC && family != collect_family)
		return;

	if (ct_entries_count >= ct_entries_max) {
		collect_lost_packets++;
		return;
	}

	switch (family) {
	case AF_INET:
		ct_entries[ct_entries_count].ce_src.ua_in = ip->ip_src;
		ct_entries[ct_entries_count].ce_dst.ua_in = ip->ip_dst;
		if (collect_proto)
			ct_entries[ct_entries_count].ce_proto = ip->ip_p;
		else
			ct_entries[ct_entries_count].ce_proto = IPPROTO_RAW;
		ct_entries[ct_entries_count].ce_bytes = ntohs(ip->ip_len);
		if ((ntohs(ip->ip_off) & IP_OFFMASK) == 0)
			p = (void *)((u_int8_t *)p + (ip->ip_hl << 2));
		else
			p = NULL;
		break;
	case AF_INET6:
		ct_entries[ct_entries_count].ce_src.ua_in6 = ip6->ip6_src;
		ct_entries[ct_entries_count].ce_dst.ua_in6 = ip6->ip6_dst;
		ct_entries[ct_entries_count].ce_proto = ip6->ip6_nxt;
		ct_entries[ct_entries_count].ce_bytes = ntohs(ip6->ip6_plen);
		p = (void *)((u_int8_t *)p + sizeof(struct ip6_hdr));
		break;
	default:
		return;
	}

	ct_entries[ct_entries_count].ce_family = family;

	if (p != NULL && collect_proto && collect_ports) {
		switch (ct_entries[ct_entries_count].ce_proto) {
		case IPPROTO_TCP:
			ct_entries[ct_entries_count].ce_sport =
				((struct tcphdr *)p)->th_sport;
			ct_entries[ct_entries_count].ce_dport =
				((struct tcphdr *)p)->th_dport;
			break;
		case IPPROTO_UDP:
			ct_entries[ct_entries_count].ce_sport =
				((struct udphdr *)p)->uh_sport;
			ct_entries[ct_entries_count].ce_dport =
				((struct udphdr *)p)->uh_dport;
			break;
		default:
			ct_entries[ct_entries_count].ce_sport = 0;
			ct_entries[ct_entries_count].ce_dport = 0;
			break;
		}
	} else {
		ct_entries[ct_entries_count].ce_sport = 0;
		ct_entries[ct_entries_count].ce_dport = 0;
	}

	if ((ce = RB_INSERT(ct_tree, &ct_head,
	    &ct_entries[ct_entries_count])) != NULL)
		ce->ce_bytes += ct_entries[ct_entries_count].ce_bytes;
	else
		ct_entries_count++;

	if (ct_entries_count >= ct_entries_max - ENTRIES_TO_SAVE)
		collect_need_dump = 1;
}

int
collect_dump(const char *interface, int need_empty_dump)
{
	char file[MAXPATHLEN];
	struct coll_header ch;
	struct ct_entry *ce;
	int fd, save_errno, dumped = 0;

	if (ct_entries_count == 0 && !need_empty_dump)
		return (0);

	if (snprintf(file, sizeof(file), CNUPM_DUMPFILE,
	    interface) >= sizeof(file)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	if ((fd = open(file, O_WRONLY | O_APPEND | O_CREAT,
	    DUMP_FILE_MODE)) < 0)
		return (-1);

	ch.ch_flags = htonl(CNUPM_VERSION);
	ch.ch_start = htonl(collect_start);
	ch.ch_stop = htonl(time(NULL));
	ch.ch_count = htonl(ct_entries_count);

	if (write(fd, &ch, sizeof(ch)) < 0) {
	error:	save_errno = errno;
		(void)close(fd);
		errno = save_errno;
		return (-1);
	}

	RB_FOREACH(ce, ct_tree, &ct_head) {
		ce->ce_bytes = htobe64(ce->ce_bytes);
		if (write(fd, &ce->ce_traffic, sizeof(ce->ce_traffic)) < 0)
			goto error;
		RB_REMOVE(ct_tree, &ct_head, ce);
		dumped++;
		 --ct_entries_count;
	}
	(void)close(fd);
	collect_need_dump = 0;
	collect_start = time(NULL);

	return (dumped);
}
