/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <net/pfvar.h>
#include <net/if_pflog.h>
#include <pcap.h>

#include "ipstatd.h"

/*
 * Packet capturing code for OpenBSD's pflog interface.
 */

#define DEF_SNAPLEN 96		/* default plus allow for larger header of pflog */
#define PFLOGD_LOG_FILE		"/var/log/pflog"
#define PFLOGD_DEFAULT_IF	"pflog0"

int snaplen = DEF_SNAPLEN;
char *filename = PFLOGD_LOG_FILE;
char *interface = PFLOGD_DEFAULT_IF;

char errbuf[PCAP_ERRBUF_SIZE];
pcap_t *hpcap;

void parse_pflog(u_char *, struct pcap_pkthdr *, u_char *);

int
open_pflog(void)
{
	pcap_t *oldhpcap = hpcap;
	int	snaplen;
	

	hpcap = pcap_open_live(interface, snaplen, 1, 500, errbuf);
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
	syslog(LOG_NOTICE, "Listening on %s, logging to %s, snaplen %d\n",
		interface, filename, snaplen);

	return (0);
}

void
read_pflog()
{
	int nump;

	while(1) {
		nump = pcap_dispatch(hpcap, 100, (pcap_handler)parse_pflog,
				    (u_char *)NULL);
	}
	return;
}

void
parse_pflog(u_char *ptr, struct pcap_pkthdr *pcaphdr, u_char *pkt)
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

