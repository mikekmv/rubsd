/*	$RuOBSD$	*/

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

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/tree.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#ifdef INET6
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
#include "collect.h"

#ifdef INET6
#define INET6_FLAGS	CNUPM_FLAG_INET6
#else
#define INET6_FLAGS	0
#endif

#ifdef PROTO
#define PROTO_FLAGS	CNUPM_FLAG_PROTO
#else
#define PROTO_FLAGS	0
#endif

#ifdef PORTS
#define PORTS_FLAGS	CNUPM_FLAG_PORTS
#else
#define PORTS_FLAGS	0
#endif

#define VERSION_FLAGS	(CNUPM_VERSION_MAJOR | (CNUPM_VERSION_MINOR << 8))
#define CNUPM_FLAGS	(INET6_FLAGS | PROTO_FLAGS | PORTS_FLAGS)

#define MIN_CT_ENTRIES	51200
#define MAX_CT_ENTRIES	102400
#define ENTRIES_TO_SAVE	256
#define DUMP_FILE_MODE	0600

struct ct_entry {
	RB_ENTRY(ct_entry)	ce_entry;
	struct coll_traffic	ce_traffic;
#ifdef INET6
#define ce_family		ce_traffic.ct_family
#endif
#ifdef PROTO
#define ce_proto		ce_traffic.ct_proto
#endif
#define ce_src			ce_traffic.ct_src
#define ce_dst			ce_traffic.ct_dst
#ifdef PORTS
#define ce_sport		ce_traffic.ct_sport
#define ce_dport		ce_traffic.ct_dport
#endif
#define ce_bytes		ce_traffic.ct_bytes
};

static int ct_entries_max = MAX_CT_ENTRIES;
static int ct_entries_count;
static time_t collect_start;
static struct ct_entry **ct_entries;
u_int32_t collect_lost_packets;
int collect_need_dump;

RB_HEAD(ct_tree, ct_entry) ct_head;

static __inline int ct_entry_compare(struct ct_entry *, struct ct_entry *);
RB_PROTOTYPE(ct_tree, ct_entry, ce_entry, ct_entry_compare);

RB_GENERATE(ct_tree, ct_entry, ce_entry, ct_entry_compare);

static __inline int
ct_entry_compare(struct ct_entry *a, struct ct_entry *b)
{
	int diff;

#ifdef PROTO
	if ((diff = a->ce_proto - b->ce_proto) != 0)
		return (diff);
#endif
#ifdef INET6
	if ((diff = a->ce_family - b->ce_family) != 0)
		return (diff);
	switch (a->ce_family) {
	case AF_INET:
#endif
		diff = a->ce_src.ua_in.s_addr - b->ce_src.ua_in.s_addr;
		if (diff != 0)
			return (diff);
		diff = a->ce_dst.ua_in.s_addr - b->ce_dst.ua_in.s_addr;
		if (diff != 0)
			return (diff);
#ifdef INET6
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
#endif	/* INET6 */
#ifdef PORTS
	if (a->ce_proto == IPPROTO_TCP || a->ce_proto == IPPROTO_UDP) {
		if ((diff = a->ce_sport - b->ce_sport) != 0)
			return (diff);
		if ((diff = a->ce_dport - b->ce_dport) != 0)
			return (diff);
	}
#endif	/* PORTS */
	return (0);
}

int
collect_init(void)
{
	int i;

	ct_entries_count = collect_lost_packets = collect_need_dump = 0;
	ct_entries = calloc(MAX_CT_ENTRIES, sizeof(struct ct_entry *));
	if (ct_entries == NULL)
		return (-1);
	for (i = 0; i < MAX_CT_ENTRIES; i++) {
		if ((ct_entries[i] = malloc(sizeof(struct ct_entry))) == NULL)
			break;
	}
	if ((ct_entries_max = i) < MIN_CT_ENTRIES) {
		for (i = 0; i < ct_entries_max; i++)
			free(ct_entries[i]);
		free(ct_entries);
		errno = ENOMEM;
		return (-1);
	}

	collect_start = time(NULL);
	return (0);
}

void
collect(sa_family_t family, const void *p)
{
	struct ip *ip = (struct ip *)p;
#ifdef INET6
	struct ip6_hdr *ip6 = (struct ip6_hdr *)p;
#endif
	struct ct_entry *ce;

	if (ct_entries_count >= ct_entries_max) {
		collect_lost_packets++;
		return;
	}

#ifdef INET6
	switch (family) {
	case AF_INET:
#endif
		ct_entries[ct_entries_count]->ce_src.ua_in = ip->ip_src;
		ct_entries[ct_entries_count]->ce_dst.ua_in = ip->ip_dst;
#ifdef PROTO
		ct_entries[ct_entries_count]->ce_proto = ip->ip_p;
#endif
		ct_entries[ct_entries_count]->ce_bytes = ntohs(ip->ip_len);
		p = (void *)((u_int8_t *)p + (ip->ip_hl << 2));
#ifdef INET6
		break;
	case AF_INET6:
		ct_entries[ct_entries_count]->ce_src.ua_in6 = ip6->ip6_src;
		ct_entries[ct_entries_count]->ce_dst.ua_in6 = ip6->ip6_dst;
#ifdef PROTO
		ct_entries[ct_entries_count]->ce_proto = ip6->ip6_nxt;
#endif
		ct_entries[ct_entries_count]->ce_bytes = ntohs(ip6->ip6_plen);
		p = (void *)((u_int8_t *)p + sizeof(struct ip6_hdr));
		break;
	default:
		return;
	}

	ct_entries[ct_entries_count]->ce_family = family;
#endif	/* INET6 */

#ifdef PORTS
	switch (ct_entries[ct_entries_count]->ce_proto) {
	case IPPROTO_TCP:
		ct_entries[ct_entries_count]->ce_sport =
			((struct tcphdr *)p)->th_sport;
		ct_entries[ct_entries_count]->ce_dport =
			((struct tcphdr *)p)->th_dport;
		break;
	case IPPROTO_UDP:
		ct_entries[ct_entries_count]->ce_sport =
			((struct udphdr *)p)->uh_sport;
		ct_entries[ct_entries_count]->ce_dport =
			((struct udphdr *)p)->uh_dport;
		break;
	default:
		ct_entries[ct_entries_count]->ce_sport = 0;
		ct_entries[ct_entries_count]->ce_dport = 0;
		break;
	}
#endif	/* PORTS */

	if ((ce = RB_INSERT(ct_tree, &ct_head,
	    ct_entries[ct_entries_count])) != NULL)
		ce->ce_bytes += ct_entries[ct_entries_count]->ce_bytes;
	else
		ct_entries_count++;

	if (ct_entries_count >= ct_entries_max - ENTRIES_TO_SAVE)
		collect_need_dump = 1;
}

int
collect_dump(const char *interface)
{
	char file[MAXPATHLEN];
	struct coll_header ch;
	struct ct_entry *ce;
	int fd, save_errno, dumped = 0;

#ifndef NEED_EMPTY_DUMP
	if (ct_entries_count == 0)
		return (0);
#endif

	if (snprintf(file, sizeof(file), CNUPM_DUMPFILE,
	    interface) >= sizeof(file)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	if ((fd = open(file, O_WRONLY | O_APPEND | O_CREAT,
	    DUMP_FILE_MODE)) < 0)
		return (-1);

	ch.ch_flags = htonl(CNUPM_FLAGS | VERSION_FLAGS);
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
	collect_need_dump = 0;
	collect_start = time(NULL);
	(void)close(fd);

	return (dumped);
}
