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
#include <machine/endian.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pcap.h>

#include "printers.h"

#define NULL_HDRLEN	4

struct printer {
	int		dlt;
	pcap_handler	printer;
};

static void print_null(u_char *, const struct pcap_pkthdr *, const u_char *);
static void print_loop(u_char *, const struct pcap_pkthdr *, const u_char *);
static void print_ether(u_char *, const struct pcap_pkthdr *, const u_char *);
static void print_packet(sa_family_t, struct ip *, u_int32_t);

static struct printer printers[] = {
	{ DLT_NULL,	print_null },
	{ DLT_LOOP,	print_loop },
	{ DLT_EN10MB,	print_ether },
	{ DLT_IEEE802,	print_ether },
	{ -1,		NULL }
};

pcap_handler
lookup_printer(int dlt)
{
	struct printer *p;

	for (p = printers; p->dlt >= 0; p++)
		if (p->dlt == dlt)
			return (p->printer);
	return (NULL);
}

static void
print_null(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	sa_family_t family = *(u_int32_t *)p;
	struct ip *ip = (struct ip *)(p + NULL_HDRLEN);

	print_packet(family, ip, h->len - NULL_HDRLEN);
}

static void
print_loop(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	*(u_int32_t *)p = htonl(*(u_int32_t *)p);
	print_null(user, h, p);
}

static void
print_ether(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	struct ether_header *ep = (struct ether_header *)p;

	p += sizeof(struct ether_header);
	switch (ntohs(ep->ether_type)) {
	case ETHERTYPE_IP:
		print_packet(AF_INET, (struct ip *)p,
		    h->len - sizeof(struct ether_header));
		break;
	case ETHERTYPE_IPV6:
		print_packet(AF_INET6, (struct ip *)p,
		    h->len - sizeof(struct ether_header));
		break;
	}
}

static void
print_packet(sa_family_t family, struct ip *ip, u_int32_t len)
{
	struct protoent *pe;
	char buf[40];
	void *ph;
	u_int32_t offset;
	u_int16_t proto;
	in_port_t sport = 0, dport = 0;
	int ports = -1;

	switch (family) {
	case AF_INET:
		(void)printf("AF_INET: ");
		if (len < sizeof(struct ip)) {
			(void)printf("shit happens\n");
			return;
		}
		offset = ip->ip_hl << 2;
		proto = ip->ip_p;
		break;
	case AF_INET6:
		(void)printf("AF_INET6: ");
		if (len < sizeof(struct ip6_hdr)) {
			(void)printf("shit happens\n");
			return;
		}
		offset = sizeof(struct ip6_hdr);
		proto = ((struct ip6_hdr *)ip)->ip6_nxt;
		break;
	default:
		(void)printf("%u\n", family);
		return;
	}

	(void)printf("proto: ");
	if ((pe = getprotobynumber(proto)) == NULL)
		(void)printf("%u, ", proto);
	else
		(void)printf("%s, ", pe->p_name);

	(void)printf("len: %u, ", (family == AF_INET ? ntohs(ip->ip_len) :
	    ntohs(((struct ip6_hdr *)ip)->ip6_plen)));

	ph = (char *)ip + offset;

	switch (ip->ip_p) {
	case IPPROTO_TCP:
		if (len < offset + sizeof(struct tcphdr))
			break;
		sport = ntohs(((struct tcphdr *)ph)->th_sport);
		dport = ntohs(((struct tcphdr *)ph)->th_dport);
		ports = 1;
		break;
	case IPPROTO_UDP:
		if (len < offset + sizeof(struct udphdr))
			break;
		sport = ntohs(((struct udphdr *)ph)->uh_sport);
		dport = ntohs(((struct udphdr *)ph)->uh_dport);
		ports = 1;
		break;
	default:
		ports = 0;
		break;
	}

	(void)printf("%s",
	    (family == AF_INET ?
	    inet_ntop(family, &ip->ip_src, buf, sizeof(buf)) :
	    inet_ntop(family, &((struct ip6_hdr *)ip)->ip6_src, buf,
	    sizeof(buf))));
	if (ports)
		(void)printf(":%u", sport);
	(void)printf(" -> %s",
	    (family == AF_INET ?
	    inet_ntop(family, &ip->ip_dst, buf, sizeof(buf)) :
	    inet_ntop(family, &((struct ip6_hdr *)ip)->ip6_dst, buf,
	    sizeof(buf))));
	if (ports)
		(void)printf(":%u", dport);

	(void)printf("\n");
}
