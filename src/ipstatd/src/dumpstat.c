/*
 *		$Id$
 */

#ifndef SOLARIS
#define SOLARIS (defined(__SVR4) || defined(__svr4__)) && defined(sun)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#if !defined(__SVR4) && !defined(__svr4__)
#include <strings.h>
#include <signal.h>
#include <sys/dir.h>
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

#define MAXENTRY        8192
#define IPLEN           16
#define BACKETLEN	64
#define MAXPORT		1024
#define LOADSTATENTRY	1024
#define KEEPLOAD_PERIOD 5

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
trafstat_t	**backet_pass,**backet_block,**backet_prn,**backet_mem_p;

u_int		ent_n=0;
u_int		*backet_len_p,*backet_pass_len;
u_int		*backet_block_len,*backet_prn_len;
int		total_packets=0,total_lines=0,total_bytes=0;

typedef struct {
	long mtype;
	char mtext[1];
	} message_t;

int usage()
{
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

int timeout()
{
	fprintf(stderr,"connection timeout \n");
	exit(1);
}

int main(argc, argv)
int argc;
char *argv[];
{
        extern  int     optind;
        extern  char    *optarg;
	int	c,n;
	char	command[16],buf[4096],auth[64],*digest;
	char	r='\n';
	time_t          timep;
        struct tm*      dt;
	int	sock_fd;
	struct sockaddr_in     	sock_server;
        struct sockaddr_in      sock_client;
	MD5_CTX 		ctx;

	fclose(stdin);
        if( (sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1 ) {
                perror("socket");
                exit(1);
        }

        sock_server.sin_family = AF_INET;
        sock_server.sin_port = htons(SERVER_PORT);
        sock_server.sin_addr.s_addr = inet_addr("194.85.82.144");

	if ( connect(sock_fd, (struct sockaddr *)&sock_server,
				 (socklen_t)sizeof(sock_server)) == -1 ) {
		perror("connect");
		exit(1);
	}
	signal(SIGALRM,(void *)&timeout);
	alarm(60);
	while ((c = getopt(argc, argv, "?hpbl")) != -1)
		switch (c) {
	                case 'p' :
				strncpy(command,"stat",sizeof(command));
				strncat(command,&r,sizeof(command));
       		                break;
                	case 'b' :
                        	break;
                	case 'l' :
                        	break;
                	default :
                	case 'h' :
                	case '?' :
                        	usage(argv[0]);
                }

	if ( (n = read(sock_fd,buf,sizeof(buf))) == -1 ) {
		perror("read");
		exit(1);
	}
#ifdef DEBUG
	printf("challenge: %s\n",buf);
#endif
	MD5Init(&ctx);
	MD5Update(&ctx, buf,strlen(buf));
	MD5Update(&ctx, password,strlen(password));
	digest = MD5End(&ctx,NULL);
	snprintf(auth,sizeof(auth),"AUTH %s\n",digest);
#ifdef DEBUG
	printf("digest: %s, len: %d\n",auth,strlen(auth));
#endif
	if ((n = write(sock_fd,auth,strlen(auth))) == -1 ) {
		perror("write");
		exit(1);
	}
	if ( (n = read(sock_fd,buf,sizeof(buf))) == -1 ) {
		perror("read");
		exit(1);
	} 
	if(!strncmp(buf,"OK\n",3))
		exit(1);
	
	if (write(sock_fd,command,strlen(command)) == -1 ) {
		perror("write");
		exit(1);
	}
	timep=time(NULL);
        dt=localtime(&timep);
        printf("\nSAMPLE %02d %02d %02d %02d %02d %02d\n\n",dt->tm_year % 100,dt
->tm_mon+1,dt->tm_mday,dt->tm_hour,dt->tm_min,dt->tm_sec);
	while ( (n = read(sock_fd,buf,sizeof(buf))) > 0 ) {
		fwrite(buf,1,n,stdout);
	}
	printf("\n");
	close(sock_fd);
}

