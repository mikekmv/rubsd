const char logpcap_ver[] = "$Id"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef pcap

#include <stddef.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/tcp_fsm.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <pcap.h>

#include "ipstat.h"
#include "ipstatd.h"

char	errbuf[PCAP_ERRBUFF_SIZE];
pcap_t	*pcapd;

void read_pcap(fd)
int	fd;
{
	int	nr = 0;
	char	buff[IPLLOGSIZE];
        char	*bp = NULL, *bpo = NULL,*buf;
        iplog_t *ipl;
        int	psize,blen;

	blen = read(fd,buff, sizeof(buff));	
	if ( blen == -1) {
		syslog(LOG_ERR,"%s: read: %m\n",iplfile);
		exit (1);
	}
	if (blen) {
		buf=buff;
		while ( blen > 0 ) {
	                ipl = (iplog_t *)buf;
        		if ((u_long)ipl & (sizeof(long)-1)) {
        			if (bp)
                			bpo = bp;
        			bp = (char *)malloc(blen);
        			bcopy((char *)ipl, bp, blen);
        			if (bpo) {
                			free(bpo);
                			bpo = NULL;
        			}
        			buf = bp;
        			continue;

        		}
        		if (ipl->ipl_magic != IPL_MAGIC) {
        			/* invalid data or out of sync */
        			break;
        		}
        		psize = ipl->ipl_dsize;
        		parse_ipchains(buf, psize);
        		blen -= psize;
        		buf += psize;
			nr++;
		}
		if (bp) {
        		free(bp);
			bp = NULL;
		}

	}
}

int parse_ipchains(buf,blen)
char    *buf;
int     blen;
{
        packdesc_t      pack;
	chainslog_t	*ipl;

        ipl = (chainslog_t *)buf;
        pack.ip = (ip_t *)((char *)ipl + sizeof(*ipf));
        pack.plen = blen - sizeof(chainslog_t);
	if(ipl->mark % 2)
        	pack.flags = FR_OUTQUE;
	else
        	pack.flags = FR_INQUE;
        pack.count = 1;
        strncpy(pack.ifname,ipl->ifname,IFNAMSIZ);

        parse_ip(&pack);
}

#endif
