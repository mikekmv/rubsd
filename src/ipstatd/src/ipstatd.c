/*	$OpenBSD: ipmon.c,v 1.18 1999/02/05 05:58:48 deraadt Exp $
 * Copyright (C) 1993-1998 by Darren Reed.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and due credit is given
 * to the original author and the contributors.
 */
#if !defined(lint)
static const char sccsid[] = "@(#)ipmon.c	1.21 6/5/96 (C)1993-1997 Darren Reed";
static const char rcsid[] = "@(#)$Id$";
#endif

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
# include <netinet/ip_fil_compat.h>
#else
# include <netinet/ip_compat.h>
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


#define MAXENTRY        8192
#define IPLEN           16
#define BACKETLEN	64
#define MAXPORT		1024
#define LOADSTATENTRY	1024
#define KEEPLOAD_PERIOD 5
#define MAX_ACT_CONN	3
#define PEER_BUF_SIZE	256

typedef struct {
		u_int	packets;
		u_int	bytes;
	} counters;

typedef struct {
	u_int	in_packets;
	u_int	in_bytes;
	u_int	out_packets;
	u_int	out_bytes;
	} miscstat_t;

time_t		start_time;

miscstat_t	*loadstat,pass_stat,block_stat;
u_int		loadstat_i = 0;

typedef struct trafstat{
                u_int	from;
                u_int	to;
                u_int   packets;
                u_int   bytes;
        } trafstat_t;

typedef	counters protostat_t;
typedef struct {
                u_int   in_from_packets;
                u_int   in_from_bytes;
                u_int   out_from_packets;
                u_int   out_from_bytes;
                u_int   in_to_packets;
                u_int   in_to_bytes;
                u_int   out_to_packets;
                u_int   out_to_bytes;
} portstat_t;

protostat_t 	*protostat;
portstat_t	*portstat_tcp, *portstat_udp;

trafstat_t	*trafstat_p,*backet_mem,*spare_backet;
trafstat_t	**backet_pass,**backet_block,**backet_prn;
trafstat_t	**bhp,**backet_mem_p;

u_int		ent_n=0;
pid_t		chpid;
u_int		*backet_len_p,*backet_pass_len;
u_int		*backet_block_len,*backet_prn_len,*blhp;
int		total_packets=0,total_lines=0,total_bytes=0;
int		nos = 0,maxsock = 0;

typedef struct {
	long 	mtype;
	char	mtext[1];
	} message_t;

void getchildstat()
{
	int	status;
	if (chpid == waitpid(chpid,&status,WNOHANG)) {
	}
}

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

void printstat(backet,backet_len)
trafstat_t	**backet;
u_int		*backet_len;
{
	int	i;
	int	packets,bytes;

	printf("\n");
	for ( i=0 ; i<256 ; i++ ) {
		print_backet(backet[i],backet_len[i]);
	}
/* Counted for each packet */
	packets = pass_stat.in_packets + pass_stat.out_packets;
	bytes = pass_stat.in_bytes + pass_stat.out_bytes;
	printf("Total packets: %d\tTotal bytes: %d\n",packets,bytes);
/* Counted from saved stats */
	printf("Total lines: %d\tCounted packets: %d\tbytes: %d\n",
					total_lines,total_packets,total_bytes);
	exit(0);
}

void write_backet_to_net(trafstat_t *full_backet, int len, int fd)
{
        int     	i;
	struct in_addr	from,to;
	char    	ip_from[IPLEN],ip_to[IPLEN];
	char		line[64];
	int		line_len;

        for ( i = 0; i < len; i++){
		total_packets += full_backet[i].packets;
		total_bytes += full_backet[i].bytes;
		from.s_addr = full_backet[i].from;
		to.s_addr = full_backet[i].to ;
		strncpy(ip_from,inet_ntoa(from),IPLEN);
		strncpy(ip_to,inet_ntoa(to),IPLEN);
                line_len = snprintf(line,sizeof(line),"%s\t%s\t%d\t%d\n",
						ip_from,ip_to,
						full_backet[i].packets,
						full_backet[i].bytes);
		if ( write(fd,line,line_len) == -1 ) {
			perror("write");
		}
	}
	total_lines += i;
}

int sendstat(backet,backet_len,fd)
trafstat_t	**backet;
u_int		*backet_len;
int		fd;
{
	int	i;

	for ( i=0 ; i<256 ; i++ ) {
		write_backet_to_net(backet[i],backet_len[i],fd);
	}
	return(0);
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

int print_protostat()
/* divide by null */
{
	int	i;
	int	bpp;		/* Bytes per packet */
	struct protoent		*proto;

	for ( i=0 ; i<256 ; i++ ) {
		if ( protostat[i].packets ) {
			bpp = protostat[i].bytes / protostat[i].packets ;
			proto = getprotobynumber(i);
			if ( proto != NULL ) {
				printf("%s:\tBytes: %d\tPackets: %d\tbpp: %d\n",					proto->p_name,protostat[i].bytes,
						protostat[i].packets,bpp);
			}else{
				printf("%d:\tBytes: %d\tPackets: %d\tbpp: %d\n",					i,protostat[i].bytes,
						protostat[i].packets,bpp);
			}
		}
	}
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
	int	fd, doread, n,i;
	int	tr, nr,mcount=0;
	char	buff[IPLLOGSIZE], *iplfile, *s;
        iplog_t *ipl;
        char *bp = NULL, *bpo = NULL,*buf,*chal;
        int psize,blen,flag;
	struct pollfd 		ipl_fds,lisn_fds;
	struct sockaddr_in	sock_server;
	char			readbuf[32];
	struct conn_state	peer[MAX_ACT_CONN];

	if ( (start_time = time(NULL)) == -1 ) {
		perror("time");
		exit(1);
	}
	
        srandom(start_time);

	iplfile = IPL_NAME;

	if( (lisn_fds.fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1 ) {
		perror("socket");
		exit(1);
	}

	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(SERVER_PORT);
	sock_server.sin_addr.s_addr = INADDR_ANY;
	if( bind(lisn_fds.fd,(struct sockaddr *)&sock_server,
					sizeof(sock_server)) == -1 ) { 
		perror("bind");
		exit(1);
	}
	if( listen(lisn_fds.fd,MAX_ACT_CONN) == -1 ) {
                perror("listen");
                exit(1);
        }



	signal(SIGCHLD, (void *)&getchildstat);
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
		if ( poll(&ipl_fds,1,100) > 0 )
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

		get_new_conn(&peer,&lisn_fds);
		serve_conn(&peer);
	}
}

get_new_conn(peer,fds)
struct conn_state *peer;
struct pollfd	*fds;
{
	int	 		i,new_sock_fd;
	struct sockaddr_in	sock_client;
	int			clnt_addr_len = sizeof(sock_client);

	fds->events = POLLIN;
	if ( poll(fds,1,0) > 0 ) {
	    if(nos < MAX_ACT_CONN)
		if ((new_sock_fd = accept(fds->fd,
			(struct sockaddr *)&sock_client,
					&clnt_addr_len)) == -1 ) {
			perror("accept");
			exit(1);
		} else {
			maxsock = MAX(maxsock,new_sock_fd);
			for ( i=0; i<MAX_ACT_CONN; i++) {
			    if (peer[i].fd == 0) {
				peer[i].buf = malloc(PEER_BUF_SIZE);
				peer[i].bufsize = PEER_BUF_SIZE;
				peer[i].fd = new_sock_fd;
				peer[i].state = START;
				peer[i].time = time(NULL);
				peer[i].rw_fl = 0;
				peer[i].rb = 0;
				nos++;
				break;
			    }
			}
#ifdef	DIAGNOSTIC
			if ( i == MAX_ACT_CONN ) {
				fprintf(stderr,"Number of open sockets: %d from MAX_ACT_CONN , but no place at peer state structure",nos);
			}
#endif
		}
	} 
}

serve_conn(peer)
struct conn_state *peer;
{
	int 		i,serr,rb;
	struct timeval	tv;
	MD5_CTX         ctx;
	fd_set		rfds,wfds,*fds;
	struct cmd	*c;
	char		*p,*cmdbuf;
	struct pollfd	tfds;


	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	for ( i=0; i<MAX_ACT_CONN ; i++) {
		if ( peer[i].fd > 0 ) {
	    	switch(peer[i].state) {
	    	    case START :
			peer[i].chal = challenge(CHAL_SIZE);
			snprintf(peer[i].buf, peer[i].bufsize,
					"CHAL %s\n",peer[i].chal);
			peer[i].nstate = WAIT_AUTH;
			peer[i].state = WRITE_DATA;
	    		peer[i].rw_fl = 0;
#ifdef DEBUG
	    		fprintf(stderr,"hello world\n");
#endif
	    		break;
		    case WAIT_AUTH :
	    		peer[i].rw_fl = 1;
			break;
	    	    case WRITE_DATA :
	    	    case READ_DATA :
	    	    case AUTHTORIZED :
	    		break;
	    	    default :
	    			/* must not occur */
#ifdef	DIAGNOSTIC
	    		fprintf(stderr,"Unknown connection state%d, state peer structure is inconsist",peer[i].state);
#endif
	    		break;
	    	}
	        fds = peer[i].rw_fl ? &rfds : &wfds ;
	        FD_SET(peer[i].fd,fds);
	    }
	}
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if ((serr = select(maxsock+1,&rfds,&wfds,NULL,&tv)) == -1 ) {
		perror("select");
	} else {
	    for ( i=0 ; i < MAX_ACT_CONN && (serr > 0) ; i++) {
		if ( peer[i].fd > 0 ) {
		   if ( FD_ISSET(peer[i].fd, &wfds)) {
			write_data_to_sock(&peer[i]);
			if( peer[i].bufload == 0 ){
				peer[i].state = peer[i].nstate;
				peer[i].rw_fl = 1;
			}
			serr--;
		   } 
		   if ( FD_ISSET(peer[i].fd, &rfds)) {
			serr--;
			rb = read(peer[i].fd,
				peer[i].buf+peer[i].rb,
				sizeof(peer[i].buf)-peer[i].rb);
			if ( rb == -1 ){
				perror("read");
				continue;
			}
			peer[i].crlfp = memchr(peer[i].buf+
					peer[i].rb,'\n',rb);
			peer[i].rb += rb ;
			if( peer[i].crlfp == NULL ) {
			    if( peer[i].rb ==
				 sizeof(peer[i].buf))
				peer[i].rb = 0;
			    continue;
			}
			peer[i].rb = 0;
			/* now in peer[i].buf string ended by \n */

			for (p = peer[i].buf; isblank(*p); p++ )
				continue;
			cmdbuf = p;
			while ( isascii(*p) && !isspace(*p) )
				p++;
			*p = '\0';
			for (c = cmdtab; c->cmdname != NULL; c++) {
                		if (!strncasecmp(c->cmdname, cmdbuf,
					 p - cmdbuf))
                        	break;
        		}
			if ( p < peer[i].crlfp)
				p++;
			while ( p < peer[i].crlfp && isblank(*p) )
                                p++;
			cmdbuf = p;
			while ( isascii(*p) && !isspace(*p) )
				p++;
			*p = '\0';
			if (c->cmdcode == AUTH_CMD ) {
				if( peer[i].state != WAIT_AUTH ) {
					get_err(INVL_ERR,&peer[i]);
					fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
					write(peer[i].fd,peer[i].buf,
							peer[i].bufload);
					close_conn(&peer[i]);
					continue;
				}
        			MD5Init(&ctx);
        			MD5Update(&ctx, peer[i].chal,
	    				strlen(peer[i].chal));
	    	        	MD5Update(&ctx, password,
	    				strlen(password));
	    			free(peer[i].chal);
	    	        	peer[i].chal = MD5End(&ctx,NULL);
#ifdef DEBUG
                        	fprintf(stderr,"digest: %s\n",peer[i].chal);
                                fprintf(stderr,"digest.recv: %s\n",cmdbuf);
#endif
				if (!strcasecmp(peer[i].chal, cmdbuf)) {
				    free(peer[i].chal);
				    get_err(OK_ERR,&peer[i]);
				    peer[i].nstate = AUTHTORIZED;
				    peer[i].state = WRITE_DATA;
				    peer[i].rw_fl = 0;
				    peer[i].rb = 0;
#ifdef DEBUG
                                    fprintf(stderr,"AUTHTORIZED\n");
#endif
				}
				continue;
			}
			if( peer[i].state < AUTHTORIZED ) {
				get_err(NAUTH_ERR,&peer[i]);
/*
				fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
				write(peer[i].fd,peer[i].buf,
							peer[i].bufload);
*/
				tfds.fd = peer[i].fd;
				tfds.events = POLLOUT;
			        if ( poll(&tfds,1,0) > 0 ) {
					write(peer[i].fd,peer[i].buf,
							peer[i].bufload);
				}
				close_conn(&peer[i]);
				break;
			}
			switch (c->cmdcode) {
			    case STAT_CMD:
				bhp = backet_prn;
				blhp = backet_prn_len;
				backet_prn = backet_pass;
				backet_prn_len = backet_pass_len;
				backet_pass = bhp;
				backet_pass_len = blhp;

				sendstat(backet_prn,
					backet_prn_len, peer[i].fd);
				memset(backet_prn_len,0,
						(256 * sizeof(int)));
				break;
			    case LOAD_CMD:
				break;
			    case MISC_CMD:
				break;
			    case PORT_CMD:
				break;
			    case PROTO_CMD:
				break;
			    case HELP_CMD:
				cmd_help(&peer[i]);
				peer[i].nstate = peer[i].state;
				peer[i].state = WRITE_DATA;
				peer[i].rw_fl = 0;
				break;
			    case QUIT_CMD:
				close_conn(&peer[i]);
				break;
			    case ERROR_CMD:
				get_err(UNKNOWN_ERR,&peer[i]);
				peer[i].nstate = peer[i].state;
				peer[i].state = WRITE_DATA;
				peer[i].rw_fl = 0;
				break;
			}
			continue;
		   } 
	        }
	    }
	}
}

int write_data_to_sock(peer)
struct conn_state	*peer;
{
	int	wb;

	wb = write(peer->fd, peer->wp,
	   		peer->bufload);
	if ( wb == -1 ) {
		perror("write");
	}
	peer->wp += wb ;
	peer->bufload -= wb ;
}

int cmd_help(peer)
struct conn_state	*peer;
{
	int 	len,n=peer->bufsize;
	struct cmd	*c;
	
	peer->wp = peer->buf;
	for (c = cmdtab; (c->cmdname != NULL) && (n > 0) ; c++) {
		len = snprintf(peer->wp,n,"%s %s\n",c->cmdname,c->cmdhelp);
		peer->wp += len;
		n -= len;
        }
	peer->bufload = peer->bufsize - n;
	peer->wp = peer->buf;
	return(peer->bufsize);
}

close_conn(peer,k)
int	k;
struct conn_state	*peer;
{
	int	i;

	free(peer[k].chal);
	free(peer[k].buf);
	if (maxsock == peer[k].fd) {
	    maxsock = 0;
	    for ( i=0 ; (i < MAX_ACT_CONN); i++)
		if( i != k )
		     maxsock = MAX(maxsock,peer[i].fd);
	}
	close(peer[k].fd);
	peer[k].fd = 0;
	nos--;
}

get_err(errnum,peer)
int	errnum;
struct conn_state	*peer;
{
	struct err		*e;
	
	peer->wp = peer->buf;
        for (e = errtab; (e->errnum != NULL) ; e++) {
	    if ( errnum == e->errnum ) {
                peer->bufload = snprintf(peer->wp,peer->bufsize,
					"%d %s\n",errnum,e->errdesc);
		return(peer->bufload);
	    }
        }
        peer->bufload = snprintf(peer->wp,peer->bufsize,
				"%d %s\n",errnum,e->errdesc);
	return(peer->bufload);
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


