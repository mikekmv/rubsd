/* 	$RuOBSD: ipstatd.c,v 1.34 2002/03/12 10:12:03 tm Exp $	*/

const char      ipstatd_ver[] = "$RuOBSD: ipstatd.c,v 1.34 2002/03/12 10:12:03 tm Exp $";

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ipstat.h"
#include "ipstatd.h"
#include "net.h"

time_t          start_time;

struct miscstat *loadstat, pass_stat, block_stat;
u_int           loadstat_i;

struct counters *protostat;
struct portstat *portstat_tcp, *portstat_udp;

struct trafstat *trafstat_p, *backet_mem, *spare_backet;
struct trafstat **backet_pass, **backet_block, **backet_prn;
struct trafstat **bhp, **backet_mem_p;
time_t          pass_time, block_time;

u_int           ent_n;
u_int          *backet_len_p, *backet_pass_len;
u_int          *backet_block_len, *backet_prn_len, *blhp;
int             total_packets, total_lines, total_bytes;
char           *myname;

#if USE_PCAP
extern struct capture pcap_cap;
struct capture	*cap = &pcap_cap;
#else
# if HAVE_PFLOG
extern struct capture pflog_cap;
struct capture	*cap = &pflog_cap;
# else
#  if HAVE_IPFILTER
extern struct capture ipl_cap;
struct capture	*cap = &ipl_cap;
#  endif
# endif
#endif

extern int      nos;
extern struct pollfd lisn_fds;
extern struct conn client[MAX_ACT_CONN];

/*
void
print_backet(struct trafstat *full_backet, int len)
{
        int     	i;
	struct in_addr	from, to;
	char    	ip_from[IPLEN], ip_to[IPLEN];

        for (i = 0; i < len; i++){
		total_packets += full_backet[i].packets;
		total_bytes += full_backet[i].bytes;
		from.s_addr = full_backet[i].from;
		to.s_addr = full_backet[i].to ;
		strncpy(ip_from, inet_ntoa(from), IPLEN);
		ip_from[IPLEN - 1] = '\0';
		strncpy(ip_to, inet_ntoa(to), IPLEN);
		ip_to[IPLEN - 1] = '\0';
                printf("%s\t%s\t%d\t%d\n", ip_from, ip_to,
		    full_backet[i].packets,
		    full_backet[i].bytes);
		if(fflush(stdout) == EOF) {
			perror("fflush");
			exit(1);
		}
        }
	total_lines += i;
}
*/

int
init_mem()
{
	int             i;

	if ((backet_mem = malloc(BACKETLEN * 256 * 3 *
				 sizeof(struct trafstat))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	if ((backet_mem_p = malloc(256 * 3 *
				   sizeof(struct trafstat *))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	if ((backet_len_p = malloc(256 * 3 *
				   sizeof(int))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	memset(backet_len_p, 0, (256 * 3 * sizeof(int)));

	backet_pass_len = backet_len_p;
	backet_block_len = backet_len_p + 256;
	backet_prn_len = backet_len_p + 2 * 256;

	backet_pass = backet_mem_p;
	backet_block = backet_mem_p + 256;
	backet_prn = backet_mem_p + 2 * 256;

	backet_pass[0] = backet_mem;
	backet_block[0] = backet_mem + BACKETLEN * 256;
	backet_prn[0] = backet_mem + BACKETLEN * 256 * 2;

	for (i = 1; i < 256; i++) {
		backet_pass[i] = backet_pass[0] + BACKETLEN * i;
		backet_block[i] = backet_block[0] + BACKETLEN * i;
		backet_prn[i] = backet_prn[0] + BACKETLEN * i;
	}

	if ((spare_backet = malloc(BACKETLEN *
				   sizeof(struct trafstat))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	if ((protostat = malloc(256 * sizeof(protostat))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	memset(protostat, 0, 256 * sizeof(protostat));

	if ((portstat_tcp =
	     malloc(MAXPORT * sizeof(struct portstat))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	memset(portstat_tcp, 0, (MAXPORT * sizeof(struct portstat)));

	if ((portstat_udp =
	     malloc(MAXPORT * sizeof(struct portstat))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	memset(portstat_udp, 0, (MAXPORT * sizeof(struct portstat)));

	memset(&pass_stat, 0, sizeof(struct miscstat));
	memset(&block_stat, 0, sizeof(struct miscstat));

	if ((loadstat =
	     malloc(LOADSTATENTRY * sizeof(struct miscstat))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		return (-1);
	}
	memset(loadstat, 0, (LOADSTATENTRY * sizeof(struct miscstat)));

	return (0);
}

int
keep_loadstat(void)
{
	loadstat_i++;
	loadstat_i &= (LOADSTATENTRY - 1);
	memcpy(&loadstat[loadstat_i], &pass_stat, sizeof(struct miscstat));
}

int
keepstat_by_proto(u_int8_t proto, u_int len)
{
	protostat[proto].packets++;
	protostat[proto].bytes += len;
}

void
sighndl(int sig)
{
	int             i;
	struct itimerval rtimer;

#ifdef	DEBUG
	if (sig != SIGALRM)
		syslog(LOG_DEBUG, "signal %d received", sig);
#endif
	switch (sig) {
	    case SIGPIPE:
		break;
	    case SIGTERM:
		stop();
		break;
	    case SIGHUP:
		break;
	    case SIGINT:
		break;
	    case SIGUSR1:
		break;
	    case SIGUSR2:
		break;
	    case SIGALRM:
		getitimer(ITIMER_REAL, &rtimer);
		for (i = 0; i < MAX_ACT_CONN; i++)
			if (client[i].fd > 0)
				client[i].timeout -= rtimer.it_interval.tv_sec;
		keep_loadstat();
		break;
	    default:
		break;
	}
}

int
main(int argc, char **argv)
{
	struct sigaction sigact;
	struct itimerval rtimer;
	sigset_t        sset;

	if ((myname = strrchr(argv[0], '/')) == NULL)
		myname = argv[0];
	else
		myname++;

	openlog(myname, LOG_PERROR, LOG_DAEMON);
	setlogmask(LOG_UPTO(LOG_DEBUG));

	pass_time = block_time = start_time = time(NULL);

	if (start_time == -1) {
		syslog(LOG_ERR, "time: %m");
	}
	srandom(start_time);

/*	sigemptyset(&sset);
	sigaddset(&sset,SIGALRM);
*/
	sigact.sa_handler = &sighndl;
	sigfillset(&sigact.sa_mask);
/*
	sigdelset(&sigact.sa_mask, SIGALRM);
	sigdelset(&sigact.sa_mask, SIGTERM);
	sigdelset(&sigact.sa_mask, SIGPIPE);
*/
	sigact.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGHUP, &sigact, NULL);
	sigaction(SIGALRM, &sigact, NULL);

	if (init_mem() == -1) {
		syslog(LOG_ERR, "Can't initialize memory, exiting...");
		exit(1);
	}
	init_net();

	openlog(myname, 0, LOG_DAEMON);
	mydaemon();
	syslog(LOG_INFO, "Starting...\n");

	cap->open();

	timerclear(&rtimer.it_interval);
	timerclear(&rtimer.it_value);
	rtimer.it_interval.tv_sec = KEEPLOAD_PERIOD;
	rtimer.it_value.tv_sec = KEEPLOAD_PERIOD;
	setitimer(ITIMER_REAL, &rtimer, NULL);

	while (1) {

		cap->read();

		lisn_fds.events = POLLIN;
		if (poll(&lisn_fds, 1, 0) > 0)
			get_new_conn(client, lisn_fds.fd);
		if (nos > 0)
			serve_conn(client);
	}
}

void
update_miscstat(u_int len, char out_fl, struct miscstat *miscstat)
{
	if (out_fl) {
		miscstat->out_packets++;
		miscstat->out_bytes += len;
	} else {
		miscstat->in_packets++;
		miscstat->in_bytes += len;
	}
}

int
keepstat_ip(int ip_from, int ip_to, int len,
	    struct trafstat **backet, u_int *backet_len)
{
	struct trafstat key;
	register struct trafstat *base;
	register int    lim, cmp;
	register struct trafstat *p;
	register int    hash;
	int             sizebuf;

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
			if (key.to > p->to)
				cmp = 1;
			else
				cmp = -1;
		} else if (key.from > p->from)
			cmp = 1;
		else
			cmp = -1;

		if (cmp > 0) {	/* key > p: move right */
			base = p + 1;
			lim--;
		}		/* else move left */
	}
	sizebuf = (char *) backet[hash] +
		backet_len[hash] * sizeof(struct trafstat) - (char *) base;
	memmove(base + 1, base, sizebuf);
	memmove(base, &key, sizeof(struct trafstat));
	++backet_len[hash];

	if (backet_len[hash] == BACKETLEN) {
		p = backet[hash];
		backet[hash] = spare_backet;
		spare_backet = p;
		backet_len[hash] = 0;
/*
	Write data to file
		print_backet(spare_backet, BACKETLEN);
*/
	}
}

int
keepstat_by_port(u_int16_t sport, u_int16_t dport,
		 u_int8_t proto, u_int len, char out_fl)
{
	u_int16_t       i;
	struct portstat *portstat;

	i = ntohs(sport);
	sport = (i < MAXPORT) ? i : 0;
	i = ntohs(dport);
	dport = (i < MAXPORT) ? i : 0;

	if (proto == IPPROTO_TCP)
		portstat = portstat_tcp;
	else
		portstat = portstat_udp;

	if (sport) {
		if (out_fl) {
			portstat[sport].out_from_packets++;
			portstat[sport].out_from_bytes += len;
		} else {
			portstat[sport].in_from_packets++;
			portstat[sport].in_from_bytes += len;
		}
	}
	if (dport) {
		if (out_fl) {
			portstat[dport].out_to_packets++;
			portstat[dport].out_to_bytes += len;
		} else {
			portstat[dport].in_to_packets++;
			portstat[dport].in_to_bytes += len;
		}
	}
}

int
parse_ip(struct packdesc *pack)
{
	struct tcphdr  *tp;
	struct icmp    *ic;
	u_short         hl, p;
	int             iplen;
	struct ip      *ip = pack->ip;
	char            out_fl, ipver;

	if (ip->ip_v != 4)	/* IPV6 not supported yet */
		return (0);

	hl = (ip->ip_hl << 2);
	p = (u_short) ip->ip_p;	/* Protocol */

	iplen = ntohs(ip->ip_len);

	/* what we must do with short ?! */
	if (pack->flags & P_SHORT) {
		return (1);
	}
	out_fl = (pack->flags & P_OUTPUT);

	if (pack->flags & P_PASS) {
/* we must process pack.count */
		update_miscstat(iplen, out_fl, &pass_stat);
		keepstat_ip(ip->ip_src.s_addr, ip->ip_dst.s_addr,
			    iplen, backet_pass, backet_pass_len);
		keepstat_by_proto(p, iplen);
		if ((p == IPPROTO_TCP || p == IPPROTO_UDP) &&
		    !(ntohs(ip->ip_off) & IP_OFFMASK)) {
/* need careful fragment analysys for precise port accounting */
			tp = (struct tcphdr *)((char *)ip + hl);
			keepstat_by_port(tp->th_sport, tp->th_dport,
					 p, iplen, out_fl);
		}
	} else if (pack->flags & P_BLOCK) {
		update_miscstat(iplen, out_fl, &block_stat);
		keepstat_ip(ip->ip_src.s_addr, ip->ip_dst.s_addr, iplen,
			    backet_block, backet_block_len);
	}

	return (0);
}
