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

#include <sys/param.h>
#include <sys/socket.h>
#ifdef __OpenBSD__
#include <sys/tree.h>
#else
#include "local/tree.h"
#endif
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "trafd.h"
#include "collect.h"

#define MIN_IC_ENTRIES		51200
#define MAX_IC_ENTRIES		102400
#define ENTRIES_TO_SAVE		256

struct ic_entry {
	RB_ENTRY(ic_entry)	ice_entry;
	struct ic_traffic	ice_traffic;
#define ice_family		ice_traffic.ict_family
#define ice_proto		ice_traffic.ict_proto
#define ice_src			ice_traffic.ict_src
#define ice_dst			ice_traffic.ict_dst
#ifndef NOPORTS
#define ice_sport		ice_traffic.ict_sport
#define ice_dport		ice_traffic.ict_dport
#endif
#define ice_bytes		ice_traffic.ict_bytes
};

extern sigset_t allsigs;

static int ic_entries_max = MAX_IC_ENTRIES;
static int ic_entries_count;
static struct ic_entry *ic_entries[MAX_IC_ENTRIES];
RB_HEAD(ic_tree, ic_entry) ic_head;

static __inline int ic_entry_compare(struct ic_entry *, struct ic_entry *);
RB_PROTOTYPE(ic_tree, ic_entry, ice_entry, ic_entry_compare);

RB_GENERATE(ic_tree, ic_entry, ice_entry, ic_entry_compare);

static __inline int
ic_entry_compare(struct ic_entry *a, struct ic_entry *b)
{
	if (a->ice_proto > b->ice_proto)
		return (1);
	if (a->ice_proto < b->ice_proto)
		return (-1);
	if (a->ice_family > b->ice_family)
		return (1);
	if (a->ice_family < b->ice_family)
		return (-1);
	switch (a->ice_family) {
	case AF_INET:
		if (a->ice_src.ica_in4.s_addr > b->ice_src.ica_in4.s_addr)
			return (1);
		if (a->ice_src.ica_in4.s_addr < b->ice_src.ica_in4.s_addr)
			return (-1);
		if (a->ice_dst.ica_in4.s_addr > b->ice_dst.ica_in4.s_addr)
			return (1);
		if (a->ice_dst.ica_in4.s_addr < b->ice_dst.ica_in4.s_addr)
			return (-1);
		break;
#ifdef INET6
	case AF_INET6:
		if (a->ice_src.ica_in6.s6_addr32[0] >
		    b->ice_src.ica_in6.s6_addr32[0])
			return (1);
		if (a->ice_src.ica_in6.s6_addr32[0] <
		    b->ice_src.ica_in6.s6_addr32[0])
			return (-1);
		if (a->ice_src.ica_in6.s6_addr32[1] >
		    b->ice_src.ica_in6.s6_addr32[1])
			return (1);
		if (a->ice_src.ica_in6.s6_addr32[1] <
		    b->ice_src.ica_in6.s6_addr32[1])
			return (-1);
		if (a->ice_src.ica_in6.s6_addr32[2] >
		    b->ice_src.ica_in6.s6_addr32[2])
			return (1);
		if (a->ice_src.ica_in6.s6_addr32[2] >
		    b->ice_src.ica_in6.s6_addr32[2])
			return (1);
		if (a->ice_src.ica_in6.s6_addr32[3] <
		    b->ice_src.ica_in6.s6_addr32[3])
			return (-1);
		if (a->ice_src.ica_in6.s6_addr32[3] >
		    b->ice_src.ica_in6.s6_addr32[3])
			return (1);
		if (a->ice_dst.ica_in6.s6_addr32[0] >
		    b->ice_dst.ica_in6.s6_addr32[0])
			return (1);
		if (a->ice_dst.ica_in6.s6_addr32[0] <
		    b->ice_dst.ica_in6.s6_addr32[0])
			return (-1);
		if (a->ice_dst.ica_in6.s6_addr32[1] >
		    b->ice_dst.ica_in6.s6_addr32[1])
			return (1);
		if (a->ice_dst.ica_in6.s6_addr32[1] <
		    b->ice_dst.ica_in6.s6_addr32[1])
			return (-1);
		if (a->ice_dst.ica_in6.s6_addr32[2] >
		    b->ice_dst.ica_in6.s6_addr32[2])
			return (1);
		if (a->ice_dst.ica_in6.s6_addr32[2] >
		    b->ice_dst.ica_in6.s6_addr32[2])
			return (1);
		if (a->ice_dst.ica_in6.s6_addr32[3] <
		    b->ice_dst.ica_in6.s6_addr32[3])
			return (-1);
		if (a->ice_dst.ica_in6.s6_addr32[3] >
		    b->ice_dst.ica_in6.s6_addr32[3])
			return (1);
		break;
#endif	/* INET6 */
	default:
		return (0);
	}
#ifndef NOPORTS
	if (a->ice_proto == IPPROTO_TCP || a->ice_proto == IPPROTO_UDP) {
		if (a->ice_sport > b->ice_sport)
			return (1);
		if (a->ice_sport < b->ice_sport)
			return (-1);
		if (a->ice_dport > b->ice_dport)
			return (1);
		if (a->ice_dport < b->ice_dport)
			return (-1);
	}
#endif	/* NOPORTS */
	return (0);
}


int
ic_init(void)
{
	int i;

	for (i = 0; i < MAX_IC_ENTRIES; i++) {
		ic_entries[i] = malloc(sizeof(struct ic_entry));
		if (ic_entries[i] == NULL)
			break;
	}
	if ((ic_entries_max = i) < MIN_IC_ENTRIES)
		return (-1);

	return (0);
}

int
ic_collect(const sa_family_t family, const void *pkt)
{
	struct ip *ip = (struct ip *)pkt;
#ifdef INET6
	struct ip6_hdr *ip6 = (struct ip6_hdr *)pkt;
#endif
	struct ic_entry *ic;

	if (ic_entries_count >= ic_entries_max)
		return (-1);

	(void)sigprocmask(SIG_BLOCK, &allsigs, NULL);

	switch (family) {
	case AF_INET:
		ic_entries[ic_entries_count]->ice_src.ica_in4 = ip->ip_src;
		ic_entries[ic_entries_count]->ice_dst.ica_in4 = ip->ip_dst;
		ic_entries[ic_entries_count]->ice_proto = ip->ip_p;
		ic_entries[ic_entries_count]->ice_bytes = ntohs(ip->ip_len);
		pkt = (void *)((u_int8_t *)pkt + (ip->ip_hl << 2));
		break;
#ifdef INET6
	case AF_INET6:
		ic_entries[ic_entries_count]->ice_src.ica_in6 = ip6->ip6_src;
		ic_entries[ic_entries_count]->ice_dst.ica_in6 = ip6->ip6_dst;
		ic_entries[ic_entries_count]->ice_proto = ip6->ip6_nxt;
		ic_entries[ic_entries_count]->ice_bytes = ntohs(ip6->ip6_plen);
		pkt = (void *)((u_int8_t *)pkt + sizeof(struct ip6_hdr));
		break;
#endif	/* INET6 */
	default:
		(void)sigprocmask(SIG_UNBLOCK, &allsigs, NULL);
		return (-1);
	}

	ic_entries[ic_entries_count]->ice_family = family;

#ifndef NOPORTS
	switch (ic_entries[ic_entries_count]->ice_proto) {
	case IPPROTO_TCP:
		ic_entries[ic_entries_count]->ice_sport =
			((struct tcphdr *)pkt)->th_sport;
		ic_entries[ic_entries_count]->ice_dport =
			((struct tcphdr *)pkt)->th_dport;
		break;
	case IPPROTO_UDP:
		ic_entries[ic_entries_count]->ice_sport =
			((struct udphdr *)pkt)->uh_sport;
		ic_entries[ic_entries_count]->ice_dport =
			((struct udphdr *)pkt)->uh_dport;
		break;
	default:
		ic_entries[ic_entries_count]->ice_sport = 0;
		ic_entries[ic_entries_count]->ice_dport = 0;
		break;
	}
#endif	/* NOPORTS */

	if ((ic = RB_INSERT(ic_tree, &ic_head,
	    ic_entries[ic_entries_count])) != NULL)
		ic->ice_bytes += ic_entries[ic_entries_count]->ice_bytes;
	else
		ic_entries_count++;

	(void)sigprocmask(SIG_UNBLOCK, &allsigs, NULL);
	return (ic_entries_count >= ic_entries_max - ENTRIES_TO_SAVE);
}

int
ic_dump(const char *device)
{
	char file[MAXPATHLEN];
	struct ic_header ih;
	struct ic_entry *ie;
	int fd, save_errno;

	if (ic_entries_count == 0)
		return (0);

	if (snprintf(file, sizeof(file), TRAFD_DUMPFILE, device) >=
	    sizeof(file)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	/* XXX: lock */
	if ((fd = open(file, O_WRONLY | O_APPEND | O_CREAT, 0600)) < 0)
		return (-1);

	ih.ich_time = time(NULL);
	ih.ich_length = ic_entries_count;

	if (write(fd, &ih, sizeof(ih)) != sizeof(ih)) {
	error:	save_errno = errno;
		(void)close(fd);
		errno = save_errno;
		return (-1);
	}

	RB_FOREACH(ie, ic_tree, &ic_head) {
		if (write(fd, &ie->ice_traffic, sizeof(ie->ice_traffic)) !=
		    sizeof(ie->ice_traffic))
			goto error;
		RB_REMOVE(ic_tree, &ic_head, ie);
		--ic_entries_count;
	}

	return (close(fd));
}
