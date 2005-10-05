/*	$RuOBSD$	*/

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

#include <sys/types.h>
#include <sys/socket.h>

#ifdef __linux__
#define __FAVOR_BSD
#endif

#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#ifdef __linux__
#include <getopt.h>
#endif
#include <limits.h>
#include <netdb.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#if defined(__FreeBSD__) || defined(__linux__)
#include <unistd.h>

#include "strtonum.h"
#endif

#ifdef __linux__
#include "strlcxx.h"
#endif


#ifndef __dead
#define __dead		__attribute__ ((__noreturn__))
#endif


#define SNAPLEN		2048


static struct sockaddr_in *addr;
static int naddr;
static int sock;

static char *interface;
static int all;
static int debug;
static int icmp;
static int ttl = IPDEFTTL;


int main(int, char *const *);
__dead static void usage(void);
static void callback(u_char *, const struct pcap_pkthdr *, const u_char *);
static u_int16_t udp_cksum(const struct ip *, const struct udphdr *);
static char *ether_sprintf(const u_char *);


int
main(int argc, char *const argv[])
{
	char ebuf[PCAP_ERRBUF_SIZE];
	u_char buf[ETHERMTU];
	char *expr = NULL, *filter;
	struct bpf_program bprog;
	pcap_t *p;
	int ch, on = 1;

	while ((ch = getopt(argc, argv, "Adf:i:It:")) != -1) {
		const char *errstr;

		switch (ch) {
		case 'A':
			all++;
			break;
		case 'd':
			debug++;
			break;
		case 'f':
			expr = optarg;
			break;
		case 'i':
			interface = optarg;
			break;
		case 'I':
			icmp++;
			break;
		case 't':
			ttl = strtonum(optarg, 1, MAXTTL, &errstr);
			if (errstr != NULL)
				errx(EX_CONFIG, "-m %s", optarg);
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		usage();

	ch = (expr == NULL || *expr == '\0' ? 0 : strlen(expr)) + 64;
	filter = malloc(ch);
	if (filter == NULL || (addr = calloc(argc, sizeof(*addr))) == NULL)
		err(EX_UNAVAILABLE, NULL);

	(void)snprintf(filter, ch,
	    "( udp %s) and %s and ( %s )", icmp ? "or icmp[0] = 8 " : "",
	    all ? "ether broadcast" : "dst 255.255.255.255",
	    expr == NULL || *expr == '\0' ? "1 = 1" : expr);

	if ((sock = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		err(EX_UNAVAILABLE, "socket");
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
		err(EX_UNAVAILABLE, "setsockopt");

	for (ch = 0; ch < argc; ch++) {
		struct hostent *h;

		if (inet_aton(argv[ch], &addr[naddr].sin_addr) != 1) {
			if ((h = gethostbyname2(argv[ch], AF_INET)) == NULL) {
				warnx("%s: %s", argv[ch], hstrerror(h_errno));
				continue;
			}
			bcopy(h->h_addr, &addr[naddr].sin_addr,
			    sizeof(addr[naddr].sin_addr));
		}
		naddr++;
	}

	if (naddr != argc) {
		void *newp;

		if ((newp = realloc(addr, sizeof(*addr) * naddr)) != NULL)
			addr = newp;
	}

	if (interface == NULL && (interface = pcap_lookupdev(ebuf)) == NULL)
		errx(EX_UNAVAILABLE, "%s", ebuf);

	if ((p = pcap_open_live(interface, SNAPLEN, 0, 1000, ebuf)) == NULL)
		errx(EX_UNAVAILABLE, "%s", ebuf);

	if ((ch = pcap_datalink(p)) != DLT_EN10MB && ch != DLT_IEEE802)
		errx(EX_CONFIG, "Unsupported interface type");

	if (pcap_compile(p, &bprog, filter, 1, 0) < 0 ||
	    pcap_setfilter(p, &bprog) < 0)
		errx(EX_CONFIG, "%s", pcap_geterr(p));

	pcap_freecode(&bprog);
	free(filter);

	if (!debug && daemon(0, 0) < 0)
		err(EX_OSERR, "daemon");

	if (debug) {
		printf("Listening on %s\n", interface);
	}

	if (pcap_loop(p, 0, callback, buf) < 0)
		errx(EX_UNAVAILABLE, "%s", pcap_geterr(p));

	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-AdI] [-f expression] "
	    "[-i interface] [-t ttl] host [...]\n", __progname);
	exit(EX_USAGE);
}

static void
callback(u_char *user, const struct pcap_pkthdr *h, const u_char *p)
{
	const struct ether_header *ep = (const struct ether_header *)p;
	const struct ip *pkt = (const struct ip *)(p + sizeof(*ep));
	struct ip *ip = (struct ip *)user;
	struct udphdr *up = (struct udphdr *)((char *)ip + (ip->ip_hl << 2));
	int i, offset;
	u_int16_t len;

	if (h->caplen < sizeof(*ep) + sizeof(*pkt) ||
	    h->caplen < (len = ntohs(pkt->ip_len)))
		return;

	bcopy(pkt, ip, len);
	offset = ntohs(ip->ip_off) & IP_OFFMASK;

	if (debug) {
		if (offset == 0 && ip->ip_p == IPPROTO_UDP) {
			printf("%s  UDP  %15s:%-5u -> ",
			    ether_sprintf(ep->ether_shost),
			    inet_ntoa(ip->ip_src), up->uh_sport);
			printf("%15s:%-5u  %u\n", inet_ntoa(ip->ip_dst),
			    up->uh_dport, len);
		} else {
			printf("%s  %s %15s       -> ",
			    ether_sprintf(ep->ether_shost),
			    ip->ip_p == IPPROTO_UDP ? "UDP " : "ICMP",
			    inet_ntoa(ip->ip_src));
			printf("%15s        %u\n", inet_ntoa(ip->ip_dst), len);
		}
	}

	for (i = 0; i < naddr; i++) {
		ssize_t n;

		ip->ip_dst.s_addr = addr[i].sin_addr.s_addr;
		ip->ip_ttl = ttl;

		if (ip->ip_p == IPPROTO_UDP && offset == 0)
			up->uh_sum = udp_cksum(ip, up);

		if (debug)
			printf("\t=> %s", inet_ntoa(ip->ip_dst));

		n = sendto(sock, ip, len, 0, (struct sockaddr *)&addr[i],
		    sizeof(addr[i]));
		if (debug) {
			if (n < 0)
				printf(": %s\n", strerror(errno));
			else
				printf("\n");
		}
	}
}

static u_int16_t
udp_cksum(const struct ip *ip, const struct udphdr *up)
{
	int i, tlen;
	union phu {
		struct phdr {
			u_int32_t src;
			u_int32_t dst;
			u_char mbz;
			u_char proto;
			u_int16_t len;
		} ph;
		u_int16_t pa[6];
	} phu;
	const u_int16_t *sp;
	u_int32_t sum;
	tlen = ntohs(ip->ip_len) - ((const char *)up - (const char*)ip);

	phu.ph.len = htons(tlen);
	phu.ph.mbz = 0;
	phu.ph.proto = ip->ip_p;
	bcopy(&ip->ip_src, &phu.ph.src, sizeof(phu.ph.src));
	bcopy(&ip->ip_dst, &phu.ph.dst, sizeof(phu.ph.dst));

	sp = &phu.pa[0];
	sum = sp[0] + sp[1] + sp[2] + sp[3] + sp[4] + sp[5];

	sp = (const u_int16_t *)up;

	for (i=0; i < (tlen & ~1); i += 2)
		sum += *sp++;

	if (tlen & 1)
		sum += htons((*(const char *)sp) << 8);

	while (sum > 0xffff)
		sum = (sum & 0xffff) + (sum >> 16);
	sum = ~sum & 0xffff;

	return (sum);
}

static char *
ether_sprintf(const u_char *ap)
{
	static char etherbuf[ETHER_ADDR_LEN * 3];
	static char digits[] = "0123456789abcdef";
	char *cp = etherbuf;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;

	return (etherbuf);
}
