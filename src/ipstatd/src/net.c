

extern char ipstatd_ver[];
const char net_ver[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef SOLARIS
#define SOLARIS (defined(__SVR4) || defined(__svr4__)) && defined(sun)
#endif

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <sys/socket.h>
#include <poll.h>

#include <stdio.h>
#include <errno.h>
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

#if defined(__OpenBSD__)
#include <netinet/ip_fil_compat.h>
#else
#include <netinet/ip_compat.h>
#endif
#include <netinet/tcpip.h>
#include <netinet/ip_fil.h>
#include <netinet/ip_proxy.h>
#include <netinet/ip_nat.h>
#include <netinet/ip_state.h>


#if	defined(sun) && !defined(SOLARIS2)
#define	STRERROR(x)	sys_errlist[x]
extern	char	*sys_errlist[];
#else
#define	STRERROR(x)	strerror(x)
#endif

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

#include "ipstat.h"
#include "ipstatd.h"
#include "net.h"

static struct err errtab[] =
{
        { OK_ERR , "- OK" },
        { AUTH_ERR , "- fake authtorization data" },
        { NAUTH_ERR , "- You are not authtorized" },
        { UNKNOWN_ERR , "- Command unknown" },
        { INVL_ERR , "- Invalid command" },
        { AUTHTMOUT_ERR , "- Authtorization timeout" },
        { TMOUT_ERR , "- Connection timeout" },
        { LOCK_ERR , "- Other client uses this resource" },
        { STOP_ERR , "- Daemon exiting..." },
        { NULL, "- Unknown error" }
};

extern trafstat_t	**bhp,**backet_prn,**backet_pass;

extern u_int	*backet_prn_len,*blhp,*backet_pass_len;
extern protostat_t	*protostat;
extern portstat_t	*portstat_tcp, *portstat_udp;
extern u_int 		loadstat_i;
extern miscstat_t	*loadstat,pass_stat,block_stat;
extern time_t		start_time,pass_time,block_time;
extern char 		*myname;

struct pollfd		lisn_fds;
struct sockaddr_in	sock_server;
conn_state	peer[MAX_ACT_CONN];
int		nos = 0,maxsock = 0;
int		statsock=0;

int write_stat_to_buf(backet,backet_len,peer)
trafstat_t	**backet;
u_int		*backet_len;
conn_state	*peer;
{
	struct in_addr	from,to;
	char    	ip_from[IPLEN],ip_to[IPLEN];
	int		len,size;
	char		*p;

	p=peer->buf+peer->bufload;
	size=peer->bufsize-peer->bufload;
	while( peer->bn < 256 ) {
	        while ( (peer->bi < backet_len[peer->bn]) && (size > 64) ){
			from.s_addr = backet[peer->bn][peer->bi].from;
			to.s_addr = backet[peer->bn][peer->bi].to ;
			strncpy(ip_from,inet_ntoa(from),IPLEN);
			strncpy(ip_to,inet_ntoa(to),IPLEN);
	                len = snprintf(p,size,"%s\t%s\t%d\t%d\n",
					ip_from,ip_to,
					backet[peer->bn][peer->bi].packets,
					backet[peer->bn][peer->bi].bytes);
			p += len;
			size -= len;
			(peer->bi)++;
		}
		if (peer->bi == backet_len[peer->bn]) {
			peer->bi = 0;
			(peer->bn)++;
		}else{
			peer->bufload = peer->bufsize - size;
			return(1);
		}
	}
	peer->bufload = peer->bufsize - size;
	peer->bn = 0;
	return(0);
}

int write_protostat_to_buf(peer)
conn_state      *peer;
{
        int     i,len,size;
	char	*p;
        int     bpp;            /* Bytes per packet */
        struct protoent         *proto;
	float	bper;

	bper = (pass_stat.out_bytes + pass_stat.in_bytes)/100;

	p=peer->buf+peer->bufload;
	size=peer->bufsize-peer->bufload;
	len = snprintf(p,size, 
		"Protocol\tBytes\t\t%%\tPackets\t\tAvgPktLen\n");
	p += len;
	size -= len;
        for ( i=0 ; i<256 && (size > 32); i++ ) {
                if ( (protostat[i].packets > 0) && bper ) {
                        bpp = protostat[i].bytes / protostat[i].packets ;
                        proto = getprotobynumber(i);
                        if ( proto != NULL ) {
                            len = snprintf(p,size,
					"%s:\t\t%-16d%.2f\t%-16d%d\n",
                                        proto->p_name,protostat[i].bytes,
						protostat[i].bytes/bper,
                                                protostat[i].packets,bpp);
                        }else{
                            len = snprintf(p,size,
					"%s:\t\t%-16d%.2f\t%-16d%d\n",
                                        i,protostat[i].bytes,
						protostat[i].bytes/bper,
                                                protostat[i].packets,bpp);
                        }
			p += len;
			size -= len;
                }
        }
	peer->bufload = peer->bufsize - size;
	return(0);
}

/* must be improved */
int write_portstat_to_buf(proto,peer)
u_int8_t	proto;
conn_state      *peer;
{
        portstat_t      *portstat;
        u_int           port;
        u_int           bpp;
	char		*protoname;
        struct servent  *portname;
        int             len,size,i;
        char            *p;
	float		bpero,bperi;

	p=peer->buf+peer->bufload;
	size=peer->bufsize-peer->bufload;
        if ( proto != IPPROTO_TCP && proto != IPPROTO_UDP ) {
                len = snprintf(p,size,"Proto must be TCP or UDP.\n");
		peer->bufload = peer->bufsize - size;
		get_err(INVLPAR_ERR,peer);
                return 1;
        }
        if ( proto == IPPROTO_TCP ) {
                portstat = portstat_tcp;
                protoname = "tcp";
        }else{
                portstat = portstat_udp;
                protoname = "udp";
        }

	bpero = pass_stat.out_bytes/100;
	bperi = pass_stat.in_bytes/100;

        len = snprintf(p,size,"Protocol: %s\n",protoname);
	p += len;
	size -= len;
        len = snprintf(p,size,
			"Port\t\t\tBytes from\t%%\tbpp\tBytes to\t%%\tbpp\n");
	p += len;
	size -= len;
        for ( port=1 ; (port<MAXPORT) && (size > 64); port++ ) {
	    if ( portstat[port].in_from_packets || 
				portstat[port].out_to_packets ) {
                len = snprintf(p,size,"%d",port);
		p += len;
		size -= len;
		i = len;
                if ( (portname = getservbyport(htons(port),
                                    protoname)) != NULL) {
                	len = snprintf(p,size," (%s)",portname->s_name);
			p += len;
			size -= len;
			i += len;
                }
		while ( i < 24 ) {
			len = snprintf(p,size,"\t");
			p += len;
			size -= len;
			i +=8;
		}
                if ( portstat[port].in_from_packets ) {
                        bpp = portstat[port].in_from_bytes / 
                                        portstat[port].in_from_packets;
                } else {
                        bpp = 0; 
		}
                len = snprintf(p,size,"%-16d%.2f\t%-8d",
                                        portstat[port].in_from_bytes,
                                        portstat[port].in_from_bytes/bperi,bpp);
		p += len;
		size -= len;
                if ( portstat[port].out_to_packets ) {
                        bpp = portstat[port].out_to_bytes / 
                                        portstat[port].out_to_packets;
                } else {
                        bpp = 0; 
		}
                len = snprintf(p,size,"%-16d%.2f\t%-8d\n",
                                        portstat[port].out_to_bytes,
                                        portstat[port].out_to_bytes/bpero,bpp);
		p += len;
		size -= len;
	    }
        }       
	peer->bufload = peer->bufsize - size;
	return 0;
}

int write_loadstat_to_buf(peer)
conn_state      *peer;
{
        u_int   age[7]={10,30,60,300,600,1800,3600};
        int     i;
        int     age_i;
        u_int   packets;
        u_int   bytes;
        u_int   bpp;            /* Bytes per packet */
        u_int   bps;            /* Bytes per second */
        u_int   pps;            /* Packets per second */
	int	len,size;
	char	*p;

	p=peer->buf+peer->bufload;
	size=peer->bufsize-peer->bufload;
        len = snprintf(p,size,
	    "Packets\t\tBytes\t\tbpp\tbps\tpps\tseconds\tInOut\n");
	p += len;
	size -= len;
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
			len = snprintf(p,size,"%-16d%-16d%d\t%d\t%d\t%d\tin\n",
                                        packets,bytes,bpp,bps,pps,age[i]);
			p += len;
			size -= len;
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
			len = snprintf(p,size,"%-16d%-16d%d\t%d\t%d\t%d\tout\n",
				 packets,bytes,bpp,bps,pps,age[i]);
			p += len;
			size -= len;
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
	len = snprintf(p,size,"%-16d%-16d%d\t%d\t%d\t%d\tinout\n",
                        packets,bytes,bpp,bps,pps,(time(NULL) - start_time));
	p += len;
	size -= len;
	peer->bufload = peer->bufsize - size;
	return(0);
}

int init_net()
{
	if( (lisn_fds.fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1 ) {
		syslog(LOG_ERR,"socket: %m, exiting...");
		exit(1);
	}

	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(SERVER_PORT);
	sock_server.sin_addr.s_addr = INADDR_ANY;
	if( bind(lisn_fds.fd,(struct sockaddr *)&sock_server,
					sizeof(sock_server)) == -1 ) { 
		syslog(LOG_ERR,"bind: %m, exiting...");
		close(lisn_fds.fd);
		exit(1);
	}
	if( listen(lisn_fds.fd,1) == -1 ) {
		syslog(LOG_ERR,"listen: %m, exiting...");
		close(lisn_fds.fd);
                exit(1);
        }
	return(0);
}

int get_new_conn(peer,fd)
conn_state *peer;
int	fd;
{
	int	 		i,new_sock_fd;
	struct sockaddr_in	sock_client;
	int			addrlen = sizeof(sock_client);

	if(nos < MAX_ACT_CONN)
	    if ((new_sock_fd = accept(fd,
	    	(struct sockaddr *)&sock_client,
	    			&addrlen)) == -1 ) {
                syslog(LOG_ERR,"listen: %m");
	    	return(-1);
	    } else {
                syslog(LOG_INFO,"Connection from: %s",
				inet_ntoa(sock_client.sin_addr));
	    	maxsock = MAX(maxsock,new_sock_fd);
	    	for ( i=0; i<MAX_ACT_CONN; i++) {
	    	    if (peer[i].fd == 0) {
	    		peer[i].rbuf = malloc(READ_BUF_SIZE);
	    		peer[i].buf = malloc(PEER_BUF_SIZE);
	    		peer[i].bufsize = PEER_BUF_SIZE;
	    		peer[i].fd = new_sock_fd;
	    		peer[i].state = START;
	    		peer[i].timeout = AUTH_TMOUT;
                        peer[i].err = OK_ERR;
	    		peer[i].rb = 0;
	    		peer[i].bn = 0;
	    		peer[i].bi = 0;
	    		nos++;
	    		break;
	    	    }
	    	}
#ifdef	DIAGNOSTIC
		if ( i == MAX_ACT_CONN ) {
			syslog(LOG_NOTICE,"Number of open sockets: %d \
			from MAX_ACT_CONN ,\n \
			but no place at peer state structure",nos);
		}
#endif
	    }
	return(0);
}

int serve_conn(peer)
conn_state *peer;
{
	int 		i,serr,rb,err;
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
			peer[i].bufload = snprintf(peer[i].buf, 
						peer[i].bufsize,
						"CHAL %s\n",peer[i].chal);
			peer[i].wp = peer[i].buf;
			peer[i].nstate = WAIT_AUTH;
			peer[i].state = WRITE_DATA;
	    		peer[i].rw_fl = 0;
#if 0
        		MD5Init(&ctx);
        		MD5Update(&ctx, peer[i].chal,
	    			strlen(peer[i].chal));
	    	        MD5Update(&ctx, password,
	    				strlen(password));
	    	        syslog(LOG_DEBUG,"AUTH %s\n", MD5End(&ctx,NULL));
#endif
	    		break;
	    	    case READ_DATA :
	    		peer[i].rw_fl = 1;
	    		break;
		    case WAIT_AUTH :
			if( peer[i].timeout <= 0 ) {
				get_err(AUTHTMOUT_ERR,&peer[i]);
				fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
				write(peer[i].fd,peer[i].buf,
						peer[i].bufload);
				p = getpeeraddr(peer[i].fd);
				if (p != NULL)
                	    	    syslog(LOG_INFO,
					"Authtorization timeout for: %s",p);
				close_conn(peer,i);
			}
	    		peer[i].rw_fl = 1;
			break;
	    	    case AUTHTORIZED :
			if( peer[i].timeout <= 0 ) {
				get_err(TMOUT_ERR,&peer[i]);
				fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
				write(peer[i].fd,peer[i].buf,
						peer[i].bufload);
				p = getpeeraddr(peer[i].fd);
				if (p != NULL)
                	    	    syslog(LOG_INFO,
					"Timeout for: %s",p);
				close_conn(peer,i);
			}
	    		peer[i].rw_fl = 1;
			break;
	    	    case WRITE_ERROR :
			get_err(peer[i].err,&peer[i]);
			peer[i].nstate = AUTHTORIZED;
                        peer[i].state = WRITE_DATA;
                        peer[i].err = OK_ERR;
	    		peer[i].rw_fl = 0;
			break;
	    	    case SEND_IP_STAT :
#ifdef	DIAGNOSTIC
			if( statsock != peer[i].fd ) {
	    		    syslog(LOG_NOTICE,"Internal statemachine error\n");
			    close_conn(peer,i);
			    exit(1);
			}
#endif
#if 0
			print_debug(&peer[i]);
#endif
			err = write_stat_to_buf(backet_prn, 
				backet_prn_len, &peer[i]);
			if(err) {
				peer[i].nstate = SEND_IP_STAT;
				peer[i].state = WRITE_DATA;
			}else{
				peer[i].nstate = CLOSE_CONN;
				peer[i].state = WRITE_DATA;
				memset(backet_prn_len,0,(256 * sizeof(int)));
				statsock = 0;
			}
#if 0
			print_debug(&peer[i]);
#endif
	    		peer[i].rw_fl = 0;
			break;
	    	    case WRITE_DATA :
	    		peer[i].rw_fl = 0;
			break;
	    	    case CLOSE_CONN :
			close_conn(peer,i);
			break;
	    	    default :
	    			/* must not occur */
#ifdef	DIAGNOSTIC
	    		syslog(LOG_NOTICE,"Unknown connection state%d, \n \
			state peer structure is inconsist",peer[i].state);
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
                syslog(LOG_ERR,"select: %m");
	} else {
	    for ( i=0 ; i < MAX_ACT_CONN && (serr > 0) ; i++) {
		if ( peer[i].fd > 0 ) {
		   if ( FD_ISSET(peer[i].fd, &wfds)) {
			err = write_data_to_sock(&peer[i]);
			if( err == -1 )
				close_conn(peer,i);
			else
			    if( peer[i].bufload == 0 ) {
			    	peer[i].wp = peer[i].buf;
			    	peer[i].state = peer[i].nstate;
			    }
			serr--;
		   } 
		   if ( FD_ISSET(peer[i].fd, &rfds)) {
			serr--;
			rb = read(peer[i].fd,
				peer[i].rbuf+peer[i].rb,
				READ_BUF_SIZE - peer[i].rb);
			if ( rb == 0 ) {
				p = getpeeraddr(peer[i].fd);
				if (p != NULL)
                	    	    syslog(LOG_INFO,
					    "Connection with %s is broken",p);
				close_conn(peer,i);
			}
			if ( rb == -1 ) {
                		syslog(LOG_ERR,"read: %m");
				continue;
			}
			peer[i].crlfp = memchr(peer[i].rbuf+
					peer[i].rb,'\n',rb);
			peer[i].rb += rb ;
			if( peer[i].crlfp == NULL ) {
			    if( peer[i].rb == READ_BUF_SIZE )
				peer[i].rb = 0;
			    continue;
			}
			peer[i].rb = 0;
			/* now in peer[i].rbuf string ended by \n */

			for (p = peer[i].rbuf; isblank(*p); p++ )
				continue;
			cmdbuf = p;
			while ( isascii(*p) && !isspace(*p) )
				p++;
			*p = '\0';
			for (c = cmdtab; c->cmdname != NULL; c++) {
#if 0
                        syslog(LOG_DEBUG,"command name: %s\n",cmdbuf);

#endif
                		if (!strncasecmp(c->cmdname, cmdbuf,
					 		strlen(c->cmdname)))
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
#if 0
                        syslog(LOG_DEBUG,"command data: %s\n",cmdbuf);
#endif
			if (c->cmdcode == AUTH_CMD ) {
				if( peer[i].state != WAIT_AUTH ) {
					get_err(INVL_ERR,&peer[i]);
					fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
					write(peer[i].fd,peer[i].buf,
							peer[i].bufload);
					peer[i].wp = peer[i].buf;
					peer[i].bufload = 0;
					close_conn(peer,i);
					continue;
				}
        			MD5Init(&ctx);
        			MD5Update(&ctx, peer[i].chal,
	    				strlen(peer[i].chal));
	    	        	MD5Update(&ctx, password,
	    				strlen(password));
	    			free(peer[i].chal);
	    	        	peer[i].chal = MD5End(&ctx,NULL);
#if 0
                        	syslog(LOG_DEBUG,"digest: %s\n",peer[i].chal);
                                syslog(LOG_DEBUG,"digest.recv: %s\n",cmdbuf);
#endif
				if (!strcasecmp(peer[i].chal, cmdbuf)) {
				    get_err(OK_ERR,&peer[i]);
				    peer[i].nstate = AUTHTORIZED;
				    peer[i].state = WRITE_DATA;
				    peer[i].timeout = READ_TMOUT;
				    peer[i].rb = 0;
#if 0
                                    syslog(LOG_DEBUG,"Client #%d is\
						 AUTHTORIZED\n",i);
#endif
				} else {
				    p = getpeeraddr(peer[i].fd);
				    if (p != NULL)
                		    	syslog(LOG_WARNING,
					    "Authtorization error for: %s",p);
				    close_conn(peer,i);
				}
				continue;
			}
			if( peer[i].state < AUTHTORIZED ) {
#ifdef DEBUG
                                syslog(LOG_DEBUG,"peer state: %d\n",
						peer[i].state);
#endif
				get_err(NAUTH_ERR,&peer[i]);
/*
				fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
				write(peer[i].fd,peer[i].buf,
							peer[i].bufload);
				peer[i].wp = peer[i].buf;
				peer[i].bufload = 0;
*/
				tfds.fd = peer[i].fd;
				tfds.events = POLLOUT;
			        if ( poll(&tfds,1,0) > 0 ) {
					write(peer[i].fd,peer[i].buf,
							peer[i].bufload);
					peer[i].wp = peer[i].buf;
					peer[i].bufload = 0;
				}
				close_conn(peer,i);
				break;
			}
			if (c->cmdcode == ERROR_CMD) {
				get_err(UNKNOWN_ERR,&peer[i]);
				peer[i].nstate = peer[i].state;
				peer[i].state = WRITE_DATA;
				continue;
			}
			peer[i].timeout = READ_TMOUT;
			switch (c->cmdcode) {
			    case STAT_CMD:
				if(statsock > 0) {
				    get_err(LOCK_ERR,&peer[i]);
				    peer[i].nstate = peer[i].state;
				    peer[i].state = WRITE_DATA;
				    break;
				}
				statsock = peer[i].fd;
				bhp = backet_prn;
				blhp = backet_prn_len;
				backet_prn = backet_pass;
				backet_prn_len = backet_pass_len;
				backet_pass = bhp;
				backet_pass_len = blhp;
				write_time_to_buf(pass_time,time(NULL),
						&peer[i]); 
				pass_time = time(NULL);
				peer[i].nstate = peer[i].state;
				peer[i].state = SEND_IP_STAT;
				break;
			    case LOAD_CMD:
				write_loadstat_to_buf(&peer[i]);
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				break;
			    case PORT_CMD:
				if( (*cmdbuf) == NULL ||
					!strncasecmp("tcp",cmdbuf,4)) {
					write_portstat_to_buf(IPPROTO_TCP,
								&peer[i]);
				}
				if( (*cmdbuf) == NULL ||
					!strncasecmp("udp",cmdbuf,4)) {
					write_portstat_to_buf(IPPROTO_UDP,
								&peer[i]);
				}
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				break;
			    case PROTO_CMD:
				write_protostat_to_buf(&peer[i]);
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				break;
			    case NOOP_CMD:
				break;
			    case HELP_CMD:
				cmd_help(&peer[i]);
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				break;
			    case QUIT_CMD:
				p = getpeeraddr(peer[i].fd);
				if (p != NULL)
                		 	syslog(LOG_INFO,
					    "%s exited",p);
				close_conn(peer,i);
				break;
			    case STOP_CMD:
				p = getpeeraddr(peer[i].fd);
				if (p != NULL)
                		 	syslog(LOG_WARNING,
					    "STOP command from: %s",p);
				stop();
				break;
#ifdef	DEBUG
                            case VERSION_CMD:
				/* remove or made get_version() */
                                strncpy(peer[i].buf,ipstatd_ver, 
					peer[i].bufsize);
				strncat(peer[i].buf,"\n",
					peer[i].bufsize - strlen(peer[i].buf));
                                strncat(peer[i].buf,net_ver, 
					peer[i].bufsize - strlen(peer[i].buf));
				strncat(peer[i].buf,"\n",
					peer[i].bufsize - strlen(peer[i].buf));
                                peer[i].bufload = strlen(peer[i].buf);
                                peer[i].nstate = WRITE_ERROR;
                                peer[i].state = WRITE_DATA;
                                break;
			    case DEBUG_CMD:
				print_debug(&peer[i]);
				break;
#endif
			}
			continue;
		   } 
	        }
	    }
	}
}

#ifdef	DEBUG
int print_debug(peer)
conn_state	*peer;
{
	syslog(LOG_DEBUG,"fd: %d\n",peer->fd);
	syslog(LOG_DEBUG,"nstate: %d\n",peer->nstate);
	syslog(LOG_DEBUG,"state: %d\n",peer->state);
	syslog(LOG_DEBUG,"rw_fl: %d\n",peer->rw_fl);
	syslog(LOG_DEBUG,"bufload: %d\n",peer->bufload);
	syslog(LOG_DEBUG,"bufsize: %d\n",peer->bufsize);
	syslog(LOG_DEBUG,"bn: %d\n",peer->bn);
	syslog(LOG_DEBUG,"bi: %d\n",peer->bi);
}
#endif

int write_data_to_sock(peer)
conn_state	*peer;
{
	int	wb;
	
	wb = write(peer->fd, peer->wp, peer->bufload);
	if ( wb == -1 ) {
                syslog(LOG_ERR,"write: %m");
		return(-1);
	}
	peer->wp += wb ;
	peer->bufload -= wb ;
}

int cmd_help(peer)
conn_state	*peer;
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

int close_conn(peer,k)
int	k;
conn_state	*peer;
{
	int	i;

	free(peer[k].chal);
	free(peer[k].buf);
	free(peer[k].rbuf);
	if (statsock == peer[k].fd)
	    statsock = 0;
	if (maxsock == peer[k].fd) {
	    maxsock = 0;
	    for ( i=0 ; (i < MAX_ACT_CONN); i++)
		if( i != k )
		     maxsock = MAX(maxsock,peer[i].fd);
	}
	i = close(peer[k].fd);
#ifdef	DEBUG
	if( i == -1)
		syslog(LOG_ERR,"close at close_conn(): %m");
#endif
	peer[k].fd = 0;
	nos--;
}

int get_err(errnum,peer)
int	errnum;
conn_state	*peer;
{
	struct err		*e;
	int			len;
	
        for (e = errtab; (e->errnum != NULL) ; e++) {
	    if ( errnum == e->errnum ) {
		break;
	    }
        }
        len = snprintf(peer->buf+peer->bufload, peer->bufsize-peer->bufload,
					"%d %s\n",errnum,e->errdesc);
	peer->bufload += len;
	return(peer->bufload);
}

int write_time_to_buf(stime,etime,peer)
time_t		stime;
time_t		etime;
conn_state	*peer;
{
	struct tm	*tm;
	int		len,size;
	char		buf[32];
	char		*p,*err;

	p=peer->buf+peer->bufload;
	size=peer->bufsize-peer->bufload;
	tm = localtime(&stime);
	strftime(buf, sizeof(buf),"%a %b %e %H:%M:%S %Z %Y",tm);
	len = snprintf(p,size,"\n%s\t",buf);
	peer->bufload += len;
	p += len;
	size -= len;
	tm = localtime(&etime);
	strftime(buf, sizeof(buf),"%a %b %e %H:%M:%S %Z %Y",tm);
	len = snprintf(p,size,"%s\n\n",buf);
	peer->bufload += len;
	return(peer->bufload);
}


void stop(void)
{
	int	i;

	i = close(lisn_fds.fd);
#ifdef	DEBUG
	if( i == -1)
		syslog(LOG_ERR,"close: %m");
#endif
	for ( i=0; i<MAX_ACT_CONN; i++) {
                if (peer[i].fd != 0) {
			get_err(STOP_ERR,&peer[i]);
			fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
			write(peer[i].fd,peer[i].buf, peer[i].bufload);
			close_conn(peer,i);
		}
	}
	/*	flush stat to disk	*/
	syslog(LOG_INFO,"%s exited.\n",myname);
	exit(0);
}

char* getpeeraddr(fd)
int	fd;
{
	int	err;
	struct sockaddr_in	sock_client;
	int			addrlen = sizeof(sock_client);

 	err = getpeername(fd,
		(struct sockaddr *)&sock_client,
			&addrlen);
	if ( err == -1 ) {
	    syslog(LOG_ERR,"getpeername: %m");
	    return(NULL);
	}
	return(inet_ntoa(sock_client.sin_addr));

}

