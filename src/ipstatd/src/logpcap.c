/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_PCAP

#if HAVE_PCAP_H
# include <pcap.h>
#else
# if HAVE_PCAP_PCAP_H
#  include <pcap/pcap.h>
# endif
#endif

#include <netinet/if_ether.h>
#include <net/ppp_defs.h>

#include "ipstatd.h"

#define SNAPLEN		128
#define TMOUT		100		/* ms */
#define IFNAME		"ppp0"

char *ifname = IFNAME;

char	errbuf[PCAP_ERRBUF_SIZE];
pcap_t	*pcapd;
int	link_type;

void parse_pcap(u_char *, struct pcap_pkthdr *, u_char *);
int  open_pcap(void);
void read_pcap(void);
void close_pcap(void);

struct capture pcap_cap = { open_pcap, read_pcap, close_pcap };

int
open_pcap()
{
	int snaplen = SNAPLEN;

	pcapd = pcap_open_live(ifname, snaplen, 0, TMOUT, errbuf);
	if (pcapd == NULL) {
		syslog(LOG_ERR, "Failed to initialize: %s\n", errbuf);
		return (-1);
	}
	link_type = pcap_datalink(pcapd);
	if (link_type != DLT_EN10MB && link_type != DLT_PPP) {
		syslog(LOG_ERR, "Unsupported datalink type\n");
		pcap_close(pcapd);
		return (-1);
	}
	snaplen = pcap_snapshot(pcapd);
	syslog(LOG_NOTICE, "Listening on %s, snaplen %d\n", ifname, snaplen);

	return (0);
}

void
read_pcap(void)
{
	int nump;

	nump = pcap_dispatch(pcapd, 1000, (pcap_handler)parse_pcap,
	    (u_char *)NULL);

	return;
}

void
parse_pcap(u_char *ptr, struct pcap_pkthdr *pcaphdr, u_char *pkt)
{
	struct packdesc	 pack;
	struct ip	*ip;
	int		 hdr_size;


	switch (link_type) {
	case DLT_EN10MB:
		hdr_size = sizeof(struct ether_header);
		break;
	case DLT_PPP:
		hdr_size = PPP_HDRLEN;
		break;
	default:
		syslog(LOG_ERR, "Unsupported datalink type\n");
		pcap_close(pcapd);
		stop();
		/* NOTREACHED */
	}

	pack.plen = pcaphdr->caplen - hdr_size;
        ip = (struct ip *)(pkt + hdr_size);

	pack.ip = ip;
	ip->ip_len = ntohs(ip->ip_len);
	ip->ip_off = ntohs(ip->ip_off);
	
	pack.flags = 0;
	pack.flags |= P_PASS;
	pack.count = 1;
	strncpy(pack.ifname, ifname, IFNAMSIZ);
	pack.ifname[IFNAMSIZ - 1] = '\0';

	parse_ip(&pack);

	return;
}

void
close_pcap(void)
{
	if (pcapd)
		pcap_close(pcapd);

	return;
}

#endif /* HAVE_PCAP */
