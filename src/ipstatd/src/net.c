
extern char ipstatd_ver[];
const char net_ver[] = "$Id$";

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

extern trafstat_t	**bhp,**backet_prn,**backet_pass;

extern u_int	*backet_prn_len,*blhp,*backet_pass_len;
extern protostat_t     *protostat;
extern portstat_t      *portstat_tcp, *portstat_udp;

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

	p=peer->buf+peer->bufload;
	size=peer->bufsize-peer->bufload;
	len = snprintf(p,size, 
		"Protocol\tBytes\tPackets\tAvgPktLen\n");
	p += len;
	size -= len;
        for ( i=0 ; i<256 && (size > 32); i++ ) {
                if ( protostat[i].packets > 0 ) {
                        bpp = protostat[i].bytes / protostat[i].packets ;
                        proto = getprotobynumber(i);
                        if ( proto != NULL ) {
                            len = snprintf(p,size, "%s:\t\t%d\t%d\t%d\n",
                                        proto->p_name,protostat[i].bytes,
                                                protostat[i].packets,bpp);
                        }else{
                            len = snprintf(p,size,"%d:\t\t%d\t%d\t%d\n",
                                        i,protostat[i].bytes,
                                                protostat[i].packets,bpp);
                        }
			p += len;
			size -= len;
                }
        }
	peer->bufload = peer->bufsize - size;
	return(0);
}

/* must be improved
int write_portstat_to_buf(proto,peer)
u_int8_t        proto;
conn_state      *peer;
{
        portstat_t      *portstat;
        u_int           port;
        u_int           bpp;
        struct servent  *portname;
        char            *protoname;
        char            line[256];
        int             line_i;
        char            *linep;

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
*/

void init_net()
{
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
	if( listen(lisn_fds.fd,1) == -1 ) {
                perror("listen");
                exit(1);
        }

}

int get_new_conn(peer,fd)
conn_state *peer;
int	fd;
{
	int	 		i,new_sock_fd;
	struct sockaddr_in	sock_client;
	int			clnt_addr_len = sizeof(sock_client);

	if(nos < MAX_ACT_CONN)
	    if ((new_sock_fd = accept(fd,
	    	(struct sockaddr *)&sock_client,
	    			&clnt_addr_len)) == -1 ) {
	    	perror("accept");
	    	return(1);
	    } else {
	    	maxsock = MAX(maxsock,new_sock_fd);
	    	for ( i=0; i<MAX_ACT_CONN; i++) {
	    	    if (peer[i].fd == 0) {
	    		peer[i].buf = malloc(PEER_BUF_SIZE);
	    		peer[i].bufsize = PEER_BUF_SIZE;
	    		peer[i].fd = new_sock_fd;
	    		peer[i].state = START;
	    		peer[i].time = time(NULL);
	    		peer[i].rb = 0;
	    		peer[i].bn = 0;
	    		peer[i].bi = 0;
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
#ifdef DEBUG
	    		fprintf(stderr,"hello world\n");
        		MD5Init(&ctx);
        		MD5Update(&ctx, peer[i].chal,
	    			strlen(peer[i].chal));
	    	        MD5Update(&ctx, password,
	    				strlen(password));
	    	        fprintf(stderr,"AUTH %s\n", MD5End(&ctx,NULL));
#endif
	    		break;
	    	    case READ_DATA :
	    		peer[i].rw_fl = 1;
	    		break;
		    case WAIT_AUTH :
			if((time(NULL) - peer[i].time) > 10) {
				get_err(AUTHTMOUT_ERR,&peer[i]);
				fcntl(peer[i].fd,F_SETFL,O_NONBLOCK);
				write(peer[i].fd,peer[i].buf,
						peer[i].bufload);
				peer[i].wp = peer[i].buf;
				peer[i].bufload = 0;
				close_conn(peer,i);
			}
	    		peer[i].rw_fl = 1;
			break;
	    	    case AUTHTORIZED :
	    		peer[i].rw_fl = 1;
			break;
	    	    case WRITE_ERROR :
			get_err(OK_ERR,&peer[i]);
			peer[i].nstate = AUTHTORIZED;
			peer[i].state = WRITE_DATA;
	    		peer[i].rw_fl = 0;
			break;
	    	    case SEND_IP_STAT :
#ifdef	DIAGNOSTIC
			if( statsock != peer[i].fd ) {
	    		    fprintf(stderr,"Internal statemachine error\n");
			    close_conn(peer,i);
			    exit(1);
			}
#endif
#ifdef DEBUG
			print_debug(&peer[i]);
#endif
			err = write_stat_to_buf(backet_prn, 
				backet_prn_len, &peer[i]);
			if(err) {
				peer[i].nstate = SEND_IP_STAT;
				peer[i].state = WRITE_DATA;
			}else{
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				memset(backet_prn_len,0,(256 * sizeof(int)));
				statsock = 0;
			}
#ifdef DEBUG
			print_debug(&peer[i]);
#endif
	    		peer[i].rw_fl = 0;
			break;
	    	    case WRITE_DATA :
	    		peer[i].rw_fl = 0;
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
				peer[i].wp = peer[i].buf;
			}
			serr--;
		   } 
		   if ( FD_ISSET(peer[i].fd, &rfds)) {
			serr--;
			rb = read(peer[i].fd,
				peer[i].buf+peer[i].rb,
				peer[i].bufsize-peer[i].rb);
			if ( rb == -1 ){
				perror("read");
				continue;
			}
			peer[i].crlfp = memchr(peer[i].buf+
					peer[i].rb,'\n',rb);
			peer[i].rb += rb ;
			if( peer[i].crlfp == NULL ) {
			    if( peer[i].rb == peer[i].bufsize)
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
#ifdef DEBUG
                        	fprintf(stderr,"cmdbuf: %s\n",cmdbuf);
                        	fprintf(stderr,"c->cmdname: %s, len %d\n",
						c->cmdname,strlen(c->cmdname));
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
#ifdef DEBUG
                        fprintf(stderr,"command data: %s\n",cmdbuf);
#endif
#ifdef DEBUG
                        fprintf(stderr,"command code: %d\n",c->cmdcode);
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
#ifdef DEBUG
                        	fprintf(stderr,"digest: %s\n",peer[i].chal);
                                fprintf(stderr,"digest.recv: %s\n",cmdbuf);
#endif
				if (!strcasecmp(peer[i].chal, cmdbuf)) {
				    get_err(OK_ERR,&peer[i]);
				    peer[i].nstate = AUTHTORIZED;
				    peer[i].state = WRITE_DATA;
				    peer[i].rb = 0;
#ifdef DEBUG
                                    fprintf(stderr,"AUTHTORIZED\n");
#endif
				}
				continue;
			}
			if( peer[i].state < AUTHTORIZED ) {
#ifdef DEBUG
                                fprintf(stderr,"peer state: %d\n",
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
				peer[i].nstate = peer[i].state;
				peer[i].state = SEND_IP_STAT;
				break;
			    case LOAD_CMD:
				break;
			    case MISC_CMD:
				break;
			    case PORT_CMD:
				break;
			    case PROTO_CMD:
				write_protostat_to_buf(&peer[i]);
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				break;
			    case HELP_CMD:
				cmd_help(&peer[i]);
				peer[i].nstate = WRITE_ERROR;
				peer[i].state = WRITE_DATA;
				break;
			    case QUIT_CMD:
				close_conn(peer,i);
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
			    case ERROR_CMD:
				get_err(UNKNOWN_ERR,&peer[i]);
				peer[i].nstate = peer[i].state;
				peer[i].state = WRITE_DATA;
				break;
			}
			continue;
		   } 
	        }
	    }
	}
}

int print_debug(peer)
conn_state	*peer;
{
	fprintf(stderr,"fd: %d\n",peer->fd);
	fprintf(stderr,"nstate: %d\n",peer->nstate);
	fprintf(stderr,"state: %d\n",peer->state);
	fprintf(stderr,"rw_fl: %d\n",peer->rw_fl);
	fprintf(stderr,"bufload: %d\n",peer->bufload);
	fprintf(stderr,"bufsize: %d\n",peer->bufsize);
	fprintf(stderr,"bn: %d\n",peer->bn);
	fprintf(stderr,"bi: %d\n",peer->bi);
}

int write_data_to_sock(peer)
conn_state	*peer;
{
	int	wb;
	
	wb = write(peer->fd, peer->wp,
	   		peer->bufload);
	if ( wb == -1 ) {
		/* log to syslog and close_conn() */
		perror("write(write_data_to_sock)");
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
	if (statsock == peer[k].fd)
	    statsock = 0;
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

