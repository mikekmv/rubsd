const char ipfil_ver[] = "$Id$";

#ifndef SOLARIS
#define SOLARIS (defined(__SVR4) || defined(__svr4__)) && defined(sun)
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <poll.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#if !defined(__SVR4) && !defined(__svr4__)
#include <strings.h>
#include <signal.h>
#else
#include <sys/filio.h>
#include <sys/byteorder.h>
#endif
#include <stdlib.h>
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

#include <sys/uio.h>
#ifndef linux
# include <sys/protosw.h>
# include <sys/user.h>
# include <netinet/ip_var.h>
#endif

#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#include <syslog.h>
#if defined(__OpenBSD__)
#include <md5.h>
#include <netinet/ip_fil_compat.h>
#else
#include <netinet/ip_compat.h>
#endif
#include <netinet/tcpip.h>
#include <netinet/ip_fil.h>
#include <netinet/ip_proxy.h>
#include <netinet/ip_nat.h>
#include <netinet/ip_state.h>

#include	"ipstatd.h"

extern	char		*iplfile;
extern	u_int		*backet_pass_len,*backet_block_len;
extern	trafstat_t	**backet_pass,**backet_block;
extern	miscstat_t	pass_stat,block_stat;

void read_ipl(fd)
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
        		parsepacket(buf, psize);
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

int parsepacket(buf,blen)
char	*buf;
int	blen;
{

        struct  protoent *pr;
        tcphdr_t        *tp;
        struct  icmp    *ic;
        struct  tm      *tm;
        char    c[3], pname[8], *t, *proto;
        u_short hl, p;
        int     i, lvl, res, len;
        ip_t    *ipc, *ip;
        iplog_t *ipl;
        ipflog_t *ipf;
	char out_fl;

        ipl = (iplog_t *)buf;
        ipf = (ipflog_t *)((char *)buf + sizeof(*ipl));
        ip = (ip_t *)((char *)ipf + sizeof(*ipf));
        hl = (ip->ip_hl << 2);
        p = (u_short)ip->ip_p;		/* Protocol */
#ifdef  linux
        ip->ip_len = ntohs(ip->ip_len);
#endif
	len = ip->ip_len;

/* what we must do with short ?! */
	if (ipf->fl_flags & FF_SHORT) {
		return(1);
	}

	out_fl = ( ipf->fl_flags & FR_OUTQUE );

#ifdef	DEBUG
	/* 	printf("out_fl %d\n",out_fl); */
#endif

        	if ( ipf->fl_flags & FR_PASS ) {
/* we must proceed ipl->ipl_count */
			update_miscstat(len,out_fl,&pass_stat);
			keepstat(ip->ip_src,ip->ip_dst,len,backet_pass,
							backet_pass_len);
			keepstat_by_proto(p,len);
			if ((p == IPPROTO_TCP || p == IPPROTO_UDP) &&
						!(ip->ip_off & IP_OFFMASK)) {
/* need more careful fragment analysys for clean port accounting */
				tp = (tcphdr_t *)((char *)ip + hl);
				keepstat_by_port(tp->th_sport,tp->th_dport,
							p,len,out_fl);
			}
        	}else{
                	if( ipf->fl_flags & FR_BLOCK ) {
				update_miscstat(len,out_fl,&block_stat);
				keepstat(ip->ip_src,ip->ip_dst,len,backet_block,
							backet_block_len);
                	}
        	}

}


