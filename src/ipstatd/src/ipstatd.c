
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

#ifdef	pcap
#include <pcap.h>
#endif

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

# if (BSD >= 199306 || linux)
static	char	*piddir = "/var/run";
# else
static	char	*piddir = "/etc";
#endif

char	*iplfile;

#include "ipstat.h"
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
char 		*myname;
extern	int	nos,maxsock,statsock;
extern	struct pollfd 		lisn_fds;
extern	conn_state	peer[MAX_ACT_CONN];
extern	char    *errbuf;

#ifdef	pcap
extern  pcap_t  *pcapd;
#endif

/*
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
*/

int init_mem()
{
	int	i;

        if( (backet_mem = malloc(BACKETLEN * 256 * 3 * 
					sizeof(trafstat_t))) == NULL ){
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
        }

        if( (backet_mem_p = malloc(256 * 3 * 
					sizeof(trafstat_t *))) == NULL ){
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
        }

        if( (backet_len_p = malloc(256 * 3 * 
					sizeof(int))) == NULL ){
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
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
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
        }

	if ( (protostat = malloc(256 * sizeof(protostat))) == NULL ) {
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
	}

	memset( protostat, 0 ,256 * sizeof(protostat));

	if ( (portstat_tcp = malloc(MAXPORT * sizeof(portstat_t))) == NULL ) {
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
	}
	
	memset(portstat_tcp,0,(MAXPORT * sizeof(portstat_t)));

	if ( (portstat_udp = malloc(MAXPORT * sizeof(portstat_t))) == NULL ) {
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
	}
	
	memset(portstat_udp,0,(MAXPORT * sizeof(portstat_t)));

	memset(&pass_stat,0,sizeof(miscstat_t));
	memset(&block_stat,0,sizeof(miscstat_t));

	if ( (loadstat = malloc(LOADSTATENTRY * sizeof(miscstat_t))) == NULL ) {
		syslog(LOG_ERR,"malloc: %m");
                return(-1);
        }
	memset(loadstat,0,(LOADSTATENTRY * sizeof(miscstat_t)));

	return(0);
}

int keep_loadstat()
{
	loadstat_i++;
	loadstat_i &= (LOADSTATENTRY - 1);
	memcpy(&loadstat[loadstat_i],&pass_stat,sizeof(miscstat_t));
	alarm(KEEPLOAD_PERIOD);
}

int keepstat_by_proto(proto,len)
u_int8_t	proto;
u_int		len;
{
	protostat[proto].packets++;
	protostat[proto].bytes += len;
}

#ifdef	DEBUG
void pipehandler(void)
{
	syslog(LOG_DEBUG,"SIGPIPE recived: %m");
}
void urghandler(void)
{
	syslog(LOG_DEBUG,"SIGURG recived: %m");
}
void huphandler(void)
{
	syslog(LOG_DEBUG,"SIGHUP recived: %m");
}
#endif

int main(argc, argv)
int argc;
char *argv[];
{
	struct	stat	sb;
	int	fd, err;
	struct pollfd 		ipl_fds;

	if((myname = strrchr(argv[0],'/')) == NULL)
          myname = argv[0];
        else
          myname++;

	openlog(myname, LOG_PERROR, LOG_DAEMON);
	setlogmask(LOG_UPTO(LOG_DEBUG));

	start_time = time(NULL);
	if ( start_time == -1 ) {
		syslog(LOG_ERR,"time: %m");
	}
        srandom(start_time);

	signal(SIGALRM,(void *)&keep_loadstat);
	signal(SIGTERM,(void *)&stop);
	if (init_mem() == -1) {
		syslog(LOG_ERR,"Can't initialize memory , exiting...");
		exit(1);
	}
	init_net();

	iplfile = IPL_NAME;

	if ((fd = open(iplfile, O_RDONLY)) == -1) {
		syslog(LOG_ERR,"%s: open: %m , exiting...",iplfile);
		exit(1);
	}
	ipl_fds.fd = fd;

#ifdef	pcap
	pcapd = pcap_open_live(ifname,10000,0,100,&errbuf);
#endif

	openlog(myname, 0, LOG_DAEMON);
	mydaemon();	
	syslog(LOG_INFO,"%s started\n",myname);
	alarm(KEEPLOAD_PERIOD);

#ifdef	DEBUG
	signal(SIGPIPE,(void *)&pipehandler);
	signal(SIGURG,(void *)&urghandler);
	signal(SIGHUP,(void *)&huphandler);
#endif

	while(TRUE) {

/* 
 *	fucking ipfilter.... poll returns 1, but read blocked :(
 *	need look at kernel...
 */
		ipl_fds.events = POLLIN;
		if ( (err = poll(&ipl_fds,1,100)) > 0 )
			read_ipl(ipl_fds.fd);
#ifdef	pcap
		read_pcap();
#endif

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

int keepstat_ip(ip_from,ip_to,len,backet,backet_len)
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
/*
	Write data to file
		print_backet(spare_backet,BACKETLEN);
*/
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

int parse_ip(pack)
packdesc_t	*pack;
{
        tcphdr_t        *tp;
        struct  icmp    *ic;
        u_short hl, p;
        int     iplen;
	ip_t	*ip = pack->ip;
	char out_fl;

        hl = (ip->ip_hl << 2);
        p = (u_short)ip->ip_p;		/* Protocol */

	iplen = ip->ip_len;

/* what we must to do with short ?! */
	if (pack->flags & FF_SHORT) {
		return(1);
	}

	out_fl = ( pack->flags & FR_OUTQUE );

        	if ( pack->flags & FR_PASS ) {
/* we must proceed pack.count */
			update_miscstat(iplen,out_fl,&pass_stat);
			keepstat_ip(ip->ip_src,ip->ip_dst,iplen,backet_pass,
							backet_pass_len);
			keepstat_by_proto(p,iplen);
			if ((p == IPPROTO_TCP || p == IPPROTO_UDP) &&
						!(ip->ip_off & IP_OFFMASK)) {
/* need more careful fragment analysys for clean port accounting */
				tp = (tcphdr_t *)((char *)ip + hl);
				keepstat_by_port(tp->th_sport,tp->th_dport,
							p,iplen,out_fl);
			}
        	}else{
                	if( pack->flags & FR_BLOCK ) {
				update_miscstat(iplen,out_fl,&block_stat);
				keepstat_ip(ip->ip_src,ip->ip_dst,iplen,
					backet_block, backet_block_len);
                	}
        	}
}

