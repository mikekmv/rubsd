/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if USE_IPCHAINS
# if HAVE_IPCHAINS
#   include <linux/ip.h>
#   include <linux/tcp.h>
#   include <linux/udp.h>
#   include <linux/icmp.h>
#   include <linux/if.h>
#   include <linux/ip_fw.h>
# endif

#include "ipstatd.h"

struct chainslog {
		int	size;
		int	mark;
		char	ifname[IFNAMSIZ];
		int32_t	unknown[3];
};

void
read_ipchains(fd)
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

int
parse_ipchains(buf,blen)
	char    *buf;
	int     blen;
{
        struct packdesc      pack;
	struct chainslog	*ipl;

        ipl = (struct chainslog *)buf;
        pack.ip = (struct ip *)((char *)ipl + sizeof(*ipf));
        pack.plen = blen - sizeof(struct chainslog);
	if(ipl->mark % 2)
        	pack.flags = FR_OUTQUE;
	else
        	pack.flags = FR_INQUE;
        pack.count = 1;
        strncpy(pack.ifname,ipl->ifname,IFNAMSIZ);
	pack.ifname[IFNAMSIZ - 1] = '\0';

        parse_ip(&pack);
}

#endif	/* USE_IPCHAINS */
