/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if USE_PCAP

# if HAVE_PCAP_H
#  include <pcap.h>
# else
#  if HAVE_PCAP_PCAP_H
#   include <pcap/pcap.h>
#  endif
# endif

#include "ipstatd.h"

void parse_pcap(u_char *, struct pcap_pkthdr *, u_char *);
int  open_pcap(void);
void read_pcap(void);
void close_pcap(void);

struct capture pcap_cap = { open_pcap, read_pcap, close_pcap };

char	errbuf[PCAP_ERRBUFF_SIZE];
pcap_t	*pcapd;

int
open_pcap()
{
	pcapd = pcap_open_live(ifname, 128, 0, 100, errbuf);
}

void
read_pcap(void)
{
	int nump;

	nump = pcap_dispatch(pcapd, 1000, (pcap_handler)parse_pcap,
	    (u_char *)NULL);

	return;
}

/* XXX: Not yet */
void
parse_pcap(u_char *ptr, struct pcap_pkthdr *pcaphdr, u_char *pkt)
{
	struct packdesc	 pack;
	struct pfloghdr	*pflog;

	pflog = (struct pfloghdr*)pkt;

        pack.ip = (struct ip *)((char *)pflog + sizeof(struct pfloghdr));
	pack.plen = pcaphdr->caplen;

	pack.flags = 0;
	if (pflog->action == PF_PASS)
		pack.flags |= P_PASS;
	else
		pack.flags |= P_BLOCK;
	if (pflog->dir == PF_OUT)
		pack.flags |= P_OUTPUT;
	pack.count = 1;
	strncpy(pack.ifname, (const char*)pflog->ifname, IFNAMSIZ);
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

#endif /* USE_PCAP */
