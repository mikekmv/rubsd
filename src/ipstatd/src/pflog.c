/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_PFLOG

# include <net/pfvar.h>
# include <net/if_pflog.h>

# if HAVE_PCAP_H
#  include <pcap.h>
# else
#  if HAVE_PCAP_PCAP_H
#   include <pcap/pcap.h>
#  endif
# endif

#endif /* HAVE_PFLOG */

#include <pcap.h>

#include "ipstatd.h"

/*
 * Packet capturing code for OpenBSD's pflog interface.
 */

#define SNAPLEN		128
#define TMOUT		100		/* ms */
#define IFNAME		"pflog0"

int snaplen = SNAPLEN;
char *interface = IFNAME;

char errbuf[PCAP_ERRBUF_SIZE];
pcap_t *hpcap;

void parse_pflog(u_char *, struct pcap_pkthdr *, u_char *);
int  open_pflog(void);
void read_pflog(void);
void close_pflog(void);

struct capture pflog_cap = { open_pflog, read_pflog, close_pflog };

int
open_pflog(void)
{
	pcap_t *oldhpcap = hpcap;

	hpcap = pcap_open_live(interface, snaplen, 0, TMOUT, errbuf);
	if (hpcap == NULL) {
		syslog(LOG_ERR, "Failed to initialize: %s\n", errbuf);
		hpcap = oldhpcap;
		return (-1);
	}
	if (pcap_datalink(hpcap) != DLT_PFLOG) {
		syslog(LOG_ERR, "Invalid datalink type\n");
		pcap_close(hpcap);
		hpcap = oldhpcap;
		return (-1);
	}
	if (oldhpcap)
		pcap_close(oldhpcap);

	snaplen = pcap_snapshot(hpcap);
	syslog(LOG_NOTICE, "Listening on %s, snaplen %d\n", interface, snaplen);

	return (0);
}

void
read_pflog(void)
{
	int nump;

	nump = pcap_dispatch(hpcap, 1000, (pcap_handler)parse_pflog,
	    (u_char *)NULL);

	return;
}

void
parse_pflog(u_char *ptr, struct pcap_pkthdr *pcaphdr, u_char *pkt)
{
	struct packdesc	 pack;
	struct pfloghdr	*pflog;
	struct ip	*ip;

	pflog = (struct pfloghdr*)pkt;

	ip = (struct ip *)((char *)pflog + sizeof(struct pfloghdr));
        pack.ip = ip;
	ip->ip_len = ntohs(ip->ip_len);
	ip->ip_off = ntohs(ip->ip_off);
	
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
close_pflog(void)
{
	if (hpcap)
		pcap_close(hpcap);

	return;
}

