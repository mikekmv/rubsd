/*	$RuOBSD: pflog.c,v 1.13 2002/03/22 17:44:10 grange Exp $	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_PFLOG

# include <net/if_pflog.h>
# include <net/pfvar.h>

# if HAVE_PCAP_H
#  include <pcap.h>
# else
#  if HAVE_PCAP_PCAP_H
#   include <pcap/pcap.h>
#  endif
# endif

#endif /* HAVE_PFLOG */

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

static void parse_pflog(u_char *, struct pcap_pkthdr *, u_char *);
static int  open_pflog(void);
static void read_pflog(void);
static void close_pflog(void);

struct capture pflog_cap = { open_pflog, read_pflog, close_pflog };

static int
open_pflog(void)
{
	int sock;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == -1) {
		syslog(LOG_ERR, "Can't create socket: %m");
		return (-1);
	}
	
	strlcpy(ifr.ifr_name, IFNAME, IFNAMSIZ); 
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
		syslog(LOG_ERR, "Unable to configure %s interface.", IFNAME);
		close(sock);
		return (-1);
	}

	/* Configure pflog0 interface if down */
	if (!(ifr.ifr_flags & IFF_UP)) {
		ifr.ifr_flags |= IFF_UP;
		if (ioctl(sock, SIOCSIFFLAGS, &ifr) == -1) {
			syslog(LOG_ERR, "Unable to configure %s interface.",
			    IFNAME);
			close(sock);
			return (-1);
		}
	}
	close(sock);

	hpcap = pcap_open_live(interface, snaplen, 0, TMOUT, errbuf);
	if (hpcap == NULL) {
		syslog(LOG_ERR, "Failed to initialize: %s", errbuf);
		return (-1);
	}
	if (pcap_datalink(hpcap) != DLT_PFLOG) {
		syslog(LOG_ERR, "Invalid datalink type");
		pcap_close(hpcap);
		return (-1);
	}
	snaplen = pcap_snapshot(hpcap);
	syslog(LOG_NOTICE, "Listening on %s, snaplen %d", interface, snaplen);

	return (0);
}

static void
read_pflog(void)
{
	int nump;

	nump = pcap_dispatch(hpcap, 1000, (pcap_handler)parse_pflog,
	    (u_char *)NULL);
}


static void
parse_pflog(u_char *ptr, struct pcap_pkthdr *pcaphdr, u_char *pkt) 
{
	struct packdesc pack;
	struct pfloghdr	*pflog;

	pflog = (struct pfloghdr*)pkt;

	pack.ip = (struct ip *)((char *)pflog + sizeof(struct pfloghdr));
	
	pack.plen = pcaphdr->caplen - sizeof(struct pfloghdr);

	pack.flags = 0;
	if (ntohs(pflog->action) == PF_PASS)
		pack.flags |= P_PASS;
	else
		pack.flags |= P_BLOCK;
	if (ntohs(pflog->dir) == PF_OUT)
		pack.flags |= P_OUTPUT;
	pack.count = 1;
	strncpy(pack.ifname, (const char*)pflog->ifname, IFNAMSIZ);
	pack.ifname[IFNAMSIZ - 1] = '\0';

	parse_ip(&pack);
}

static void
close_pflog(void)
{
	int sock;
	struct ifreq ifr;

	if (hpcap)
		pcap_close(hpcap);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
		return;

	strlcpy(ifr.ifr_name, IFNAME, IFNAMSIZ); 

	/* Unconfigure pflog0 interface */
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) != -1) {
		ifr.ifr_flags &= ~IFF_UP;
		ioctl(sock, SIOCSIFFLAGS, &ifr);
	}
	close(sock);
}

