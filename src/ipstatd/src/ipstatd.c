const char ipstatd_ver[] = "$Id$";

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

#include <ctype.h>
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

#include "ipstat.h"

#if	defined(sun) && !defined(SOLARIS2)
#define	STRERROR(x)	sys_errlist[x]
extern	char	*sys_errlist[];
#else
#define	STRERROR(x)	strerror(x)
#endif


struct	flags {
	int	value;
	char	flag;
};

struct	flags	tcpfl[] = {
	{ TH_ACK, 'A' },
	{ TH_RST, 'R' },
	{ TH_SYN, 'S' },
	{ TH_FIN, 'F' },
	{ TH_URG, 'U' },
	{ TH_PUSH,'P' },
	{ 0, '\0' }
};

#if SOLARIS
static	char	*pidfile = "/etc/opt/ipf/ipmon.pid";
#else
# if (BSD >= 199306 || linux)
static	char	*pidfile = "/var/run/ipstatd.pid";
# else
static	char	*pidfile = "/etc/ipmon.pid";
# endif
#endif

struct hlist {
	struct hlist *next;
	struct in_addr addr;
	char name[MAXHOSTNAMELEN];
};

#define PRIME 367
static  struct	hlist htable[PRIME];

#include "ipstatd.h"
#include "net.h"

time_t		start_time;

miscstat_t	*loadstat,pass_stat,block_stat;
u_int		loadstat_i = 0;

protostat_t 	*protostat;
portstat_t	*portstat_tcp, *portstat_udp;

trafstat_t	*trafstat_p,*backet_mem,*spare_backet;
trafstat_t	**backet_pass,**backet_block,**backet_prn;
trafstat_t	**bhp,**backet_mem_p;

u_int		ent_n=0;
u_int		*backet_len_p,*backet_pass_len;
u_int		*backet_block_len,*backet_prn_len,*blhp;
int		total_packets=0,total_lines=0,total_bytes=0;
extern	int	nos,maxsock,statsock;
extern	struct pollfd 		lisn_fds;
extern	conn_state	peer[MAX_ACT_CONN];

void print_backet(trafstat_t *full_backet, int len)
{
        int     	i;
	struct in_addr	from,to;
	char    	ip_from[IPLEN],ip_to[IPLEN];

        for ( i = 0; i < len; i++){
		total_packets += full_backet[i].packets;
		total_bytes += full_backet[i].bytes;
		from.s_addr = full_backet[i].from;
		to.s_addr = full_backet[i].to ;
		strncpy(ip_from,inet_ntoa(from),IPLEN);
		strncpy(ip_to,inet_ntoa(to),IPLEN);
                printf("%s\t%s\t%d\t%d\n",ip_from,ip_to,
						full_backet[i].packets,
						full_backet[i].bytes);
		if( fflush(stdout) == EOF ) {
			perror("fflush");
			exit(1);
		}
        }
	total_lines += i;
}

void init_mem()
{
	int	i;

        if( (backet_mem = malloc(BACKETLEN * 256 * 3 * 
					sizeof(trafstat_t))) == NULL ){
                perror("malloc");
                exit(1);
        }

        if( (backet_mem_p = malloc(256 * 3 * 
					sizeof(trafstat_t *))) == NULL ){
                perror("malloc");
                exit(1);
        }

        if( (backet_len_p = malloc(256 * 3 * 
					sizeof(int))) == NULL ){
                perror("malloc");
                exit(1);
        }
     	
	memset(backet_len_p,0,(256 * 3 * sizeof(int)));

	backet_pass_len = backet_len_p;
	backet_block_len = backet_len_p + 256;
	backet_prn_len = backet_len_p + 2 * 256;

	backet_pass = backet_mem_p;
	backet_block = backet_mem_p + 256;
	backet_prn = backet_mem_p + 2 * 256;

	backet_pass[0] = backet_mem;
	backet_block[0] = backet_mem + BACKETLEN * 256;
	backet_prn[0] = backet_mem + BACKETLEN * 256 * 2;

	for ( i = 1; i<256 ; i++) {
		backet_pass[i] = backet_pass[0] + BACKETLEN * i;
		backet_block[i] = backet_block[0] + BACKETLEN * i;
		backet_prn[i] = backet_prn[0] + BACKETLEN * i;
	}

	if( (spare_backet = malloc(BACKETLEN * 
					sizeof(trafstat_t))) == NULL ){
                perror("malloc");
                exit(1);
        }

	if ( (protostat = malloc(256 * sizeof(protostat))) == NULL ) {
		perror("malloc");
                exit(1);
	}

	memset( protostat, 0 ,256 * sizeof(protostat));

	if ( (portstat_tcp = malloc(MAXPORT * sizeof(portstat_t))) == NULL ) {
		perror("malloc");
                exit(1);
	}
	
	memset(portstat_tcp,0,(MAXPORT * sizeof(portstat_t)));

	if ( (portstat_udp = malloc(MAXPORT * sizeof(portstat_t))) == NULL ) {
		perror("malloc");
                exit(1);
	}
	
	memset(portstat_udp,0,(MAXPORT * sizeof(portstat_t)));

	memset(&pass_stat,0,sizeof(miscstat_t));
	memset(&block_stat,0,sizeof(miscstat_t));

	if ( (loadstat = malloc(LOADSTATENTRY * sizeof(miscstat_t))) == NULL ) {
                perror("malloc");
                exit(1);
        }
	memset(loadstat,0,(LOADSTATENTRY * sizeof(miscstat_t)));

}

int keep_loadstat()
{
	loadstat_i++;
	loadstat_i &= (LOADSTATENTRY - 1);
	memcpy(&loadstat[loadstat_i],&pass_stat,sizeof(miscstat_t));
	alarm(KEEPLOAD_PERIOD);
}

int print_loadstat()
/* Need more check about " divide by zero" */
{
	u_int	age[7]={10,30,60,300,600,1800,3600};
	int	i;
	int	age_i;
	u_int	packets;
	u_int	bytes;
	u_int	bpp;		/* Bytes per packet */
	u_int	bps;		/* Bytes per second */
	u_int	pps;		/* Packets per second */

	for ( i=0 ; i<7 ; i++) {
		age_i = (LOADSTATENTRY + loadstat_i - age[i]/KEEPLOAD_PERIOD) 
							& (LOADSTATENTRY - 1);
		if ( loadstat[age_i].in_packets ) {
			packets = loadstat[loadstat_i].in_packets - 
					loadstat[age_i].in_packets;
			bytes = loadstat[loadstat_i].in_bytes - 
				loadstat[age_i].in_bytes;
			if ( packets ) {
				bpp = bytes / packets;
				bps = bytes / age[i];
				pps = packets / age[i];
			}else{
				bpp = bps = pps = 0;
			}
			printf("Incoming traffic\n");
			printf("Packets \tBytes \tbpp \tbps \tpps \tseconds\n");
			printf("%d \t\t%d \t%d \t%d \t%d \t%d\n",
					packets,bytes,bpp,bps,pps,age[i]);
		}
		if ( loadstat[age_i].out_packets ) {
			packets = loadstat[loadstat_i].out_packets - 
					loadstat[age_i].out_packets;
			bytes = loadstat[loadstat_i].out_bytes - 
					loadstat[age_i].out_bytes;
			if ( packets ) {
				bpp = bytes / packets;
				bps = bytes / age[i];
				pps = packets / age[i];
			}else{
				bpp = bps = pps = 0;
			}
			printf("Outgoing traffic\n");
			printf("Packets \tBytes \tbpp \tbps \tpps \tseconds\n");
			printf("%d \t\t%d \t%d \t%d \t%d \t%d\n",
					packets,bytes,bpp,bps,pps,age[i]);
		}
	}
	packets = pass_stat.out_packets + pass_stat.in_packets;
	bytes = pass_stat.out_bytes + pass_stat.in_bytes;
	if ( packets ) {
		bpp = bytes / packets;
		bps = bytes / (time(NULL) - start_time);
		pps = packets / (time(NULL) - start_time);
	}else{
		bpp = bps = pps = 0;
	}
	printf("Full traffic\n");
	printf("Packets \tBytes \tbpp \tbps \tpps \tseconds\n");
	printf("%d \t\t%d \t%d \t%d \t%d \t%d\n",
			packets,bytes,bpp,bps,pps,(time(NULL) - start_time));

print_portstat(IPPROTO_TCP);
}

int keepstat_by_proto(proto,len)
u_int8_t	proto;
u_int		len;
{
	protostat[proto].packets++;
	protostat[proto].bytes += len;
}

/* must be improved */
int print_portstat(proto)
u_int8_t	proto;
{
	portstat_t      *portstat;
	u_int		port;
	u_int		bpp;
	struct servent	*portname;
	char		*protoname;
	char		line[256];
	int		line_i;
	char		*linep;

	if ( proto != IPPROTO_TCP && proto != IPPROTO_UDP ) {
		fprintf(stderr,"Proto must be TCP or UDP.\n");
		return 1;
	}
	if ( proto == IPPROTO_TCP ) {
		portstat = portstat_tcp;
		protoname = "tcp";
	}else{
		portstat = portstat_udp;
		protoname = "udp";
	}

	printf("Port\tBytes from\tbpp\tBytes to\tbpp\n");
	for ( port=1 ; port<MAXPORT ; port++ ) {
		line_i = sizeof(line);
		linep = line;
		*linep = '\0';
		if ( portstat[port].in_from_packets ) {
			bpp = portstat[port].in_from_bytes / 
					portstat[port].in_from_packets;
			line_i -= snprintf(linep,line_i,"\t %d\t %d",
					portstat[port].in_from_bytes,bpp);
			if ( line_i > 0 )
				linep += strlen(linep);
		}
		if ( portstat[port].out_to_packets ) {
			bpp = portstat[port].out_to_bytes / 
					portstat[port].out_to_packets;
			line_i -= snprintf(linep,line_i,"\t %d\t %d",
					portstat[port].out_to_bytes,bpp);
			if ( line_i > 0 )
				linep += strlen(linep);
		}
		if ( strlen(line) ) {
			printf("%d ",port);
			if ( (portname = getservbyport(htons(port),
							protoname)) != NULL) {
				printf("( %s ):",portname->s_name);
			}
			printf("%s\n",line);
				
		}
	}	
}

int main(argc, argv)
int argc;
char *argv[];
{
	struct	stat	sb;
	int	fd, doread,err;
	int	tr, nr;
	char	buff[IPLLOGSIZE], *iplfile;
        iplog_t *ipl;
        char *bp = NULL, *bpo = NULL,*buf;
        int psize,blen;
	struct pollfd 		ipl_fds;

	if ( (start_time = time(NULL)) == -1 ) {
		perror("time");
		exit(1);
	}
	
        srandom(start_time);

	init_net();

	iplfile = IPL_NAME;

	signal(SIGALRM,(void *)&keep_loadstat);

	init_mem();
	keep_loadstat();

	if ((fd = open(iplfile, O_RDONLY)) == -1) {
		(void) fprintf(stderr,
			       "%s: open: %s\n", iplfile,
			       STRERROR(errno));
		exit(1);
	}
	ipl_fds.fd = fd;

	for (doread = 1; doread; ) {
		nr = 0;
		tr = 0;
		
		ipl_fds.events = POLLIN;
/* 
 *	fucking ipfilter.... poll returns 1, but read blocked :(
 *	need look at kernel...
 */
		if ( (err = poll(&ipl_fds,1,100)) > 0 )
		    if (( blen = read(fd,buff, sizeof(buff))) == -1) {
			perror("read");
			exit (1);
		    }
/*
printf("blen = %d\n",blen);
*/
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

		lisn_fds.events = POLLIN;
		if ( poll(&lisn_fds,1,0) > 0 )
			get_new_conn(peer,lisn_fds.fd);
		if(nos > 0)
			serve_conn(peer);
	}
}

update_miscstat(len,out_fl,miscstat)
u_int	len;
char	out_fl;
miscstat_t	*miscstat;
{
	if ( out_fl ) {
			miscstat->out_packets++;
			miscstat->out_bytes += len;
	}else{
			miscstat->in_packets++;
			miscstat->in_bytes += len;
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

int keepstat(ip_from,ip_to,len,backet,backet_len)
int		ip_from;
int		ip_to;
int		len;
trafstat_t	**backet;
u_int		*backet_len;
{
	trafstat_t	key;
        register trafstat_t *base;
        register int lim, cmp;
        register trafstat_t *p;
	register int	hash;
	int	sizebuf;

	key.from = ip_from;
	key.to = ip_to;
	key.packets = 1;
	key.bytes = len;

	hash = ntohl(ip_from ^ ip_to) & 0xff;
/*
	hash = (u_int)(ip_from ^ ip_to) >> 24;
*/
	base = backet[hash];
        for (lim = backet_len[hash]; lim != 0; lim >>= 1) {
                p = base + (lim >> 1);
                cmp = key.from - p->from;
		if (cmp == 0) {
			cmp = key.to - p->to;
                	if (cmp == 0) {
				p->packets++;
				p->bytes += len;
                        	return (0);
			}
			if ( key.to > p->to )
				cmp = 1;
			else
				cmp = -1;
		}else
			if ( key.from > p->from )
				cmp = 1;
			else
				cmp = -1;

                if (cmp > 0) {  /* key > p: move right */
                        base = p + 1;
                        lim--;
                } /* else move left */
        }
	sizebuf = (char *)backet[hash] + backet_len[hash]*sizeof(trafstat_t) - 
								(char *)base ;
	memmove(base+1,base,sizebuf);
	memmove(base,&key,sizeof(trafstat_t));
	++backet_len[hash];

	if ( backet_len[hash] == BACKETLEN ) {
		p = backet[hash];
		backet[hash] = spare_backet;
		spare_backet = p;
		backet_len[hash] = 0;
		print_backet(spare_backet,BACKETLEN);
	}
}

int keepstat_by_port(sport,dport,proto,len,out_fl)
u_int16_t	sport;
u_int16_t	dport;
u_int8_t	proto;
u_int		len;
char		out_fl;
{
	u_int16_t	i;
	portstat_t	*portstat;
	
	i = htons(sport);
	sport = ( i<MAXPORT ) ? i : 0;
	i = htons(dport);
	dport = ( i<MAXPORT ) ? i : 0;

	if ( proto == IPPROTO_TCP )
		portstat = portstat_tcp;
	else
		portstat = portstat_udp;

	if ( sport ) {
		if ( out_fl ) {
			portstat[sport].out_from_packets++;
			portstat[sport].out_from_bytes += len;
		}else{
			portstat[sport].in_from_packets++;
			portstat[sport].in_from_bytes += len;
		}
	}
	if ( dport ) {
		if ( out_fl ) {
			portstat[dport].out_to_packets++;
			portstat[dport].out_to_bytes += len;
		}else{
			portstat[dport].in_to_packets++;
			portstat[dport].in_to_bytes += len;
		}
	}
}


