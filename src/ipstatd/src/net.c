/*	$RuOBSD: net.c,v 1.29 2002/12/02 13:34:44 tm Exp $	*/

extern char ipstatd_ver[];
const char net_ver[] = "$RuOBSD: net.c,v 1.29 2002/12/02 13:34:44 tm Exp $";

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "extern.h"
#include "ipstat.h"
#include "ipstatd.h"
#include "net.h"

static struct cmd cmdtab[] =
{
	{"auth", "- MD5 sum for authtorization", AUTH_CMD},
	{"stat", "- generic IP statistic", STAT_CMD},
	{"load", "- get load statistic", LOAD_CMD},
	{"port", " [udp|tcp] - get port traffik statistic", PORT_CMD},
	{"proto", "- get protocol statistic", PROTO_CMD},
	{"help", "- this help", HELP_CMD},
	{"?", "- this help", HELP_CMD},
	{"nop", "- no operation", NOP_CMD},
	{"quit", "- close connection", QUIT_CMD},
	{"stop", "- close all connections and exit daemon", STOP_CMD},
	{"version", "- Get version info", VERSION_CMD},
#ifdef  DEBUG
	{"debug", "- Print internal vars", DEBUG_CMD},
#endif
	{NULL, "", ERROR_CMD}
};

static struct err errtab[] = {
	{OK_ERR, "OK"},
	{AUTH_ERR, "fake authtorization data"},
	{NAUTH_ERR, "You are not authtorized"},
	{UNKNOWN_ERR, "Command unknown"},
	{INVL_ERR, "Invalid command"},
	{AUTHTMOUT_ERR, "Authtorization timeout"},
	{TMOUT_ERR, "Connection timeout"},
	{LOCK_ERR, "Other client uses this resource"},
	{STOP_ERR, "Daemon exiting..."},
	{NULL, "Unknown error"}
};

extern struct trafstat **bhp, **bucket_prn, **bucket_pass;

extern u_int *bucket_prn_len, *blhp, *bucket_pass_len;
extern struct counters *protostat;
extern struct portstat *portstat_tcp, *portstat_udp;
extern u_int loadstat_i;
extern struct miscstat *loadstat, pass_stat, block_stat;
extern time_t start_time, pass_time, block_time;
extern char *myname;
extern struct capture *cap;

struct pollfd lisn_fds;
struct sockaddr_in sock_server;
struct conn client[MAX_ACT_CONN];
int nos, maxsock;
int statsock = -1;

int
write_stat_to_buf (struct trafstat **bucket, u_int *bucket_len, struct conn *client)
{
	struct in_addr from, to;
	char ip_from[IPLEN], ip_to[IPLEN];
	int len, size;
	char *p;

	p = client->buf + client->bufload;
	size = client->bufsize - client->bufload;
	while (client->bn < 256) {
		while (client->bi < bucket_len[client->bn]) {
			from.s_addr = bucket[client->bn][client->bi].from;
			to.s_addr = bucket[client->bn][client->bi].to;
			len = strlcpy(ip_from, inet_ntoa(from), sizeof(ip_from));
			if (len >= IPLEN)	/* paranoia */
				break;
			len = strlcpy(ip_to, inet_ntoa(to), sizeof(ip_to));
			if (len >= IPLEN)	/* paranoia */
				break;
			len = snprintf(p, size, "%s\t%s\t%llu\t%llu\n",
			    ip_from, ip_to,
			    bucket[client->bn][client->bi].packets,
			    bucket[client->bn][client->bi].bytes);
			if (len >= size)
				break;
			p += len;
			size -= len;
			client->bi++;
		}
		if (client->bi == bucket_len[client->bn]) {
			client->bi = 0;
			client->bn++;
		} else {
			client->bufload = client->bufsize - size;
			return (1);
		}
	}
	client->bufload = client->bufsize - size;
	client->bn = 0;
	return (0);
}

int
write_protostat_to_buf(struct conn *client)
{
	int i, len, size;
	char *p;
	u_int64_t bpp;		/* Bytes per packet */
	struct protoent *proto;
	float bper;

	bper = (pass_stat.out_bytes + pass_stat.in_bytes) / 100;

	p = client->buf + client->bufload;
	size = client->bufsize - client->bufload;
	len = snprintf(p, size,
		       "Protocol\tBytes\t\t%%\tPackets\t\tAvgPktLen\n");
	if (len >= size)
		return (1);	/* handling return value not implemented yet */
	p += len;
	size -= len;
	for (i = 0; i < 256 && (size > 32); i++) {
		if ((protostat[i].packets > 0) && bper) {
			bpp = protostat[i].bytes / protostat[i].packets;
			proto = getprotobynumber(i);
			if (proto != NULL) {
				len = snprintf(p, size,
				    "%s:\t\t%-16llu%.2f\t%-16llu%llu\n",
				    proto->p_name, protostat[i].bytes,
				    protostat[i].bytes / bper,
				    protostat[i].packets, bpp);
			} else {
				len = snprintf(p, size,
				    "%d:\t\t%-16llu%.2f\t%-16llu%llu\n",
				    i, protostat[i].bytes,
				    protostat[i].bytes / bper,
				    protostat[i].packets, bpp);
			}
			/* handling return value not implemented yet */
			if (len >= size)
				return (1);
			p += len;
			size -= len;
		}
	}
	client->bufload = client->bufsize - size;
	return (0);
}

/* must be improved */
int
write_portstat_to_buf(u_int8_t proto, struct conn *client)
{
	struct portstat *portstat;
	u_int port;
	u_int64_t bpp;
	char *protoname;
	struct servent *portname;
	int len, size, i;
	char *p;
	float bpero, bperi;

	p = client->buf + client->bufload;
	size = client->bufsize - client->bufload;
	
	switch (proto) {
	case IPPROTO_TCP:
		portstat = portstat_tcp;
		protoname = "tcp";
		break;
	case IPPROTO_UDP:
		portstat = portstat_udp;
		protoname = "udp";
		break;
	default:
		/* hm... is it better to return error via another buffer ? */
		len = snprintf(p, size, "Proto should be a TCP or UDP.\n");
		/*
		if (len >= size)
			return (1);
		*/
		client->bufload = client->bufsize - size;
		get_err(INVLPAR_ERR, client);
		return (1);
	}

	bpero = pass_stat.out_bytes / 100;
	bperi = pass_stat.in_bytes / 100;

	len = snprintf(p, size, "Protocol: %s\n", protoname);
	if (len >= size)
		return (1);
	p += len;
	size -= len;
	len = snprintf(p, size,
		"Port\t\t\tBytes from\t%%\tbpp\tBytes to\t%%\tbpp\n");
	if (len >= size)
		return (1);
	p += len;
	size -= len;
	for (port = 1; port < MAXPORT; port++) {
		if (portstat[port].in_from_packets
		    || portstat[port].out_to_packets) {
			len = snprintf(p, size, "%u", port);
			if (len >= size)
				return (1);
			p += len;
			size -= len;
			i = len;
			if ((portname = getservbyport(htons(port),
			    protoname)) != NULL) {
				len = snprintf(p, size, " (%s)",
					portname->s_name);
				if (len >= size)
					return (1);
				p += len;
				size -= len;
				i += len;
			}
			while (i < 24) {
				len = snprintf(p, size, "\t");
				if (len >= size)
					return (1);
				p += len;
				size -= len;
				i += 8;
			}
			if (portstat[port].in_from_packets) {
				bpp = portstat[port].in_from_bytes /
					portstat[port].in_from_packets;
			} else {
				bpp = 0;
			}
			len = snprintf(p, size, "%-16llu%.2f\t%-8llu",
				portstat[port].in_from_bytes,
				portstat[port].in_from_bytes / bperi, bpp);
			if (len >= size)
				return (1);
			p += len;
			size -= len;
			if (portstat[port].out_to_packets) {
				bpp = portstat[port].out_to_bytes /
					portstat[port].out_to_packets;
			} else {
				bpp = 0;
			}
			len = snprintf(p, size, "%-16llu%.2f\t%-8llu\n",
				portstat[port].out_to_bytes,
				portstat[port].out_to_bytes / bpero, bpp);
			if (len >= size)
				return (1);
			p += len;
			size -= len;
		}
	}
	client->bufload = client->bufsize - size;
	return (0);
}

int
write_loadstat_to_buf(struct conn *client)
{
	u_int age[7] = {10, 30, 60, 300, 600, 1800, 3600};
	int i;
	int age_i;
	u_int packets;
	u_int bytes;
	u_int bpp;	/* Bytes per packet */
	u_int bps;	/* Bytes per second */
	u_int pps;	/* Packets per second */
	int len, size;
	char *p;

	p = client->buf + client->bufload;
	size = client->bufsize - client->bufload;
	len = snprintf(p, size,
		"Packets\t\tBytes\t\tbpp\tbps\tpps\tseconds\tInOut\n");
	if (len >= size)
		return (1);
	p += len;
	size -= len;
	for (i = 0; i < 7; i++) {
		age_i = (LOADSTATENTRY + loadstat_i - age[i] / KEEPLOAD_PERIOD)
			& (LOADSTATENTRY - 1);
		if (loadstat[age_i].in_packets) {
			packets = loadstat[loadstat_i].in_packets -
				loadstat[age_i].in_packets;
			bytes = loadstat[loadstat_i].in_bytes -
				loadstat[age_i].in_bytes;
			if (packets) {
				bpp = bytes / packets;
				bps = bytes / age[i];
				pps = packets / age[i];
			} else {
				bpp = bps = pps = 0;
			}
			len = snprintf(p, size,
				"%-16u%-16u%u\t%u\t%u\t%u\tin\n",
				packets, bytes, bpp, bps, pps, age[i]);
			if (len >= size)
				return (1);
			p += len;
			size -= len;
		}
		if (loadstat[age_i].out_packets) {
			packets = loadstat[loadstat_i].out_packets -
				loadstat[age_i].out_packets;
			bytes = loadstat[loadstat_i].out_bytes -
				loadstat[age_i].out_bytes;
			if (packets) {
				bpp = bytes / packets;
				bps = bytes / age[i];
				pps = packets / age[i];
			} else {
				bpp = bps = pps = 0;
			}
			len = snprintf(p, size,
				"%-16u%-16u%u\t%u\t%u\t%u\tout\n",
				packets, bytes, bpp, bps, pps, age[i]);
			if (len >= size)
				return (1);
			p += len;
			size -= len;
		}
	}
	packets = pass_stat.out_packets + pass_stat.in_packets;
	bytes = pass_stat.out_bytes + pass_stat.in_bytes;
	if (packets) {
		bpp = bytes / packets;
		bps = bytes / (time(NULL) - start_time);
		pps = packets / (time(NULL) - start_time);
	} else {
		bpp = bps = pps = 0;
	}
	len = snprintf(p, size, "%-16u%-16u%u\t%u\t%u\t%u\tin+out\n",
		packets, bytes, bpp, bps, pps, (time(NULL) - start_time));
	if (len >= size)
		return (1);
	p += len;
	size -= len;
	client->bufload = client->bufsize - size;
	return (0);
}

int
init_net(void)
{
	int i;

	if ((lisn_fds.fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		syslog(LOG_ERR, "socket: %m");
		return -1;
	}
	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(SERVER_PORT);
	sock_server.sin_addr.s_addr = INADDR_ANY;
	if (bind(lisn_fds.fd, (struct sockaddr *)&sock_server,
		 sizeof(sock_server)) == -1) {
		syslog(LOG_ERR, "bind: %m");
		close(lisn_fds.fd);
		return -1;
	}
	if (listen(lisn_fds.fd, 1) == -1) {
		syslog(LOG_ERR, "listen: %m");
		close(lisn_fds.fd);
		return -1;
	}

	for (i = 0; i < MAX_ACT_CONN; i++)
		client[i].fd = -1;
	
	return (0);
}

int
get_new_conn(struct conn *client, int fd)
{
	int i, new_sock_fd;
	struct sockaddr_in sock_client;
	int addrlen = sizeof(sock_client);

	if (nos < MAX_ACT_CONN) {
		if ((new_sock_fd = accept(fd, (struct sockaddr *)&sock_client,
		    &addrlen)) == -1) {
			syslog(LOG_ERR, "listen: %m");
			return (-1);
		} else {
			syslog(LOG_INFO, "Connection from: %s",
			       inet_ntoa(sock_client.sin_addr));
			maxsock = MAX(maxsock, new_sock_fd);
			for (i = 0; i < MAX_ACT_CONN; i++) {
				if (client[i].fd == -1) {
					client[i].rbuf = malloc(READ_BUF_SIZE);
					client[i].buf = malloc(PEER_BUF_SIZE);
					client[i].bufsize = PEER_BUF_SIZE;
					client[i].fd = new_sock_fd;
					client[i].state = START;
					client[i].timeout = AUTH_TMOUT;
					client[i].err = OK_ERR;
					client[i].rb = 0;
					client[i].bn = 0;
					client[i].bi = 0;
					nos++;
					break;
				}
			}
#ifdef DIAGNOSTIC
			if (i == MAX_ACT_CONN) {
				syslog(LOG_NOTICE, "Number of open sockets: %d"
				    " from %d,\n but no place"
				    " at client state structure",
				    nos, MAX_ACT_CONN);
			}
#endif
		}
	}

	return (0);
}

int
write_time_to_buf(time_t stime, time_t etime, struct conn *client)
{
	struct tm *tm;
	int len, size;
	char buf[32];
	char *p;

	p = client->buf + client->bufload;
	size = client->bufsize - client->bufload;
	tm = localtime(&stime);
	strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Z %Y", tm);
	len = snprintf(p, size, "\n%s\t", buf);
	if (len >= size)
		return (1);
	client->bufload += len;
	p += len;
	size -= len;
	tm = localtime(&etime);
	strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Z %Y", tm);
	len = snprintf(p, size, "%s\n\n", buf);
	if (len >= size)
		return (1);
	client->bufload += len;

	return (client->bufload);
}

int
serve_conn(struct conn *client)
{
	int i, serr, rb, err;
	struct timeval tv;
	MD5_CTX ctx;
	fd_set rfds, wfds, *fds;
	struct cmd *c;
	char *p, *cmdbuf;
	struct pollfd tfds;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	for (i = 0; i < MAX_ACT_CONN; i++) {
		if (client[i].fd >= 0) {
			switch (client[i].state) {
			case START:
			    client[i].chal = challenge(CHAL_SIZE);
			    client[i].bufload = snprintf(client[i].buf,
						       client[i].bufsize,
					     "CHAL %s\n", client[i].chal);
			    client[i].wp = client[i].buf;
			    client[i].nstate = WAIT_AUTH;
			    client[i].state = WRITE_DATA;
			    client[i].rw_fl = 0;
#if DEBUG
			    MD5Init(&ctx);
			    MD5Update(&ctx, client[i].chal,
				      strlen(client[i].chal));
			    MD5Update(&ctx, password,
				      strlen(password));
			    syslog(LOG_DEBUG, "AUTH %s\n", MD5End(&ctx, NULL));
#endif
			    break;
			case READ_DATA:
			    client[i].rw_fl = 1;
			    break;
			case WAIT_AUTH:
			    if (client[i].timeout <= 0) {
				    get_err(AUTHTMOUT_ERR, &client[i]);
				    fcntl(client[i].fd, F_SETFL, O_NONBLOCK);
				    write(client[i].fd, client[i].buf,
					  client[i].bufload);
				    p = getclientaddr(client[i].fd);
				    if (p != NULL)
					    syslog(LOG_INFO,
						"Authtorization "
						"timeout for: %s", p);
				    close_conn(client, i);
				    continue;
			    }
			    client[i].rw_fl = 1;
			    break;
			case AUTHTORIZED:
			    if (client[i].timeout <= 0) {
				    get_err(TMOUT_ERR, &client[i]);
				    fcntl(client[i].fd, F_SETFL, O_NONBLOCK);
				    write(client[i].fd, client[i].buf,
					  client[i].bufload);
				    p = getclientaddr(client[i].fd);
				    if (p != NULL)
					    syslog(LOG_INFO,
						   "Timeout for: %s", p);
				    close_conn(client, i);
				    continue;
			    }
			    client[i].rw_fl = 1;
			    break;
			case WRITE_ERROR:
			    get_err(client[i].err, &client[i]);
			    client[i].nstate = AUTHTORIZED;
			    client[i].state = WRITE_DATA;
			    client[i].err = OK_ERR;
			    client[i].rw_fl = 0;
			    break;
			case SEND_IP_STAT:
#ifdef	DIAGNOSTIC
			    if (statsock != client[i].fd) {
				    syslog(LOG_ERR,
					"Internal state machine error\n");
				    /* XXX: We must dump core here */
				    close_conn(client, i);
				    exit(1);
			    }
#endif
#if DEBUG
			    print_debug(&client[i]);
#endif
			    err = write_stat_to_buf(bucket_prn,
					      bucket_prn_len, &client[i]);
			    if (err) {
				    client[i].nstate = SEND_IP_STAT;
				    client[i].state = WRITE_DATA;
			    } else {
				    client[i].nstate = CLOSE_CONN;
				    client[i].state = WRITE_DATA;
				    memset(bucket_prn_len, 0, (256 * sizeof(int)));
				    statsock = -1;
			    }
#if DEBUG
			    print_debug(&client[i]);
#endif
			    client[i].rw_fl = 0;
			    break;
			case WRITE_DATA:
			    client[i].rw_fl = 0;
			    break;
			case CLOSE_CONN:
			    close_conn(client, i);
			    continue;
			default:
			    /* must not occure */
#ifdef	DIAGNOSTIC
			    syslog(LOG_NOTICE, "Unknown connection state %d,"
				" client structure in inconsistent state\n",
				client[i].state);
#endif
			    break;
			}
			fds = client[i].rw_fl ? &rfds : &wfds;
			FD_SET(client[i].fd, fds);
		}
	}
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if ((serr = select(maxsock + 1, &rfds, &wfds, NULL, &tv)) == -1) {
		syslog(LOG_ERR, "select: %m");
	} else {
	    for (i = 0; i < MAX_ACT_CONN && (serr > 0); i++) {
		if (client[i].fd >= 0) {
			if (FD_ISSET(client[i].fd, &wfds)) {
				serr--;
				err = write_data_to_sock(&client[i]);
				if (err == -1) {
					close_conn(client, i);
					continue;
				} else if (client[i].bufload == 0) {
					client[i].wp = client[i].buf;
					client[i].state = client[i].nstate;
				}
			}
			if (FD_ISSET(client[i].fd, &rfds)) {
				serr--;
				rb = read(client[i].fd,
					  client[i].rbuf + client[i].rb,
					READ_BUF_SIZE - client[i].rb);
				if (rb == 0) {
					p = getclientaddr(client[i].fd);
					if (p != NULL)
						syslog(LOG_INFO,
						    "Connection with %s is broken", p);
					close_conn(client, i);
					continue;
				}
				if (rb == -1) {
					syslog(LOG_ERR, "read: %m");
					continue;
				}
				client[i].crlfp = memchr(client[i].rbuf +
					      client[i].rb, '\n', rb);
				client[i].rb += rb;
				if (client[i].crlfp == NULL) {
					if (client[i].rb == READ_BUF_SIZE)
						client[i].rb = 0;
					continue;
				}
				client[i].rb = 0;

				/*
				 * now in client[i].rbuf string ended
				 * by \n
				 */

				for (p = client[i].rbuf; isblank(*p); p++)
					continue;
				cmdbuf = p;
				while (isascii(*p) && !isspace(*p))
					p++;
				*p = '\0';
				for (c = cmdtab; c->cmdname != NULL; c++) {
#if DEBUG
					syslog(LOG_DEBUG, "command name: %s\n", cmdbuf);

#endif
					if (!strncasecmp(c->cmdname, cmdbuf,
						strlen(c->cmdname)))
						break;
				}
				if (p < client[i].crlfp)
					p++;
				while (p < client[i].crlfp && isblank(*p))
					p++;
				cmdbuf = p;
				while (isascii(*p) && !isspace(*p) && *p != '\0' && *p != '\n')
					p++;
				*p = '\0';
#if DEBUG
				syslog(LOG_DEBUG, "command data: %s\n", cmdbuf);
#endif
				if (c->cmdcode == AUTH_CMD) {
					if (client[i].state != WAIT_AUTH) {
						get_err(INVL_ERR, &client[i]);
						fcntl(client[i].fd, F_SETFL,
						    O_NONBLOCK);
						write(client[i].fd,
						    client[i].buf,
						    client[i].bufload);
						client[i].wp = client[i].buf;
						client[i].bufload = 0;
						close_conn(client, i);
						continue;
					}
					MD5Init(&ctx);
					MD5Update(&ctx, client[i].chal,
					      strlen(client[i].chal));
					MD5Update(&ctx, password,
						  strlen(password));
					free(client[i].chal);
					client[i].chal = MD5End(&ctx, NULL);
#if DEBUG
					syslog(LOG_DEBUG, "digest: %s\n",
					    client[i].chal);
					syslog(LOG_DEBUG, "digest.recv: %s\n",
					    cmdbuf);
#endif
					if (!strcasecmp(client[i].chal,
					    cmdbuf)) {
						get_err(OK_ERR, &client[i]);
						client[i].nstate = AUTHTORIZED;
						client[i].state = WRITE_DATA;
						client[i].timeout = READ_TMOUT;
						client[i].rb = 0;
#if DEBUG
						syslog(LOG_DEBUG, "Client #%d"
						    " is AUTHTORIZED\n", i);
#endif
					} else {
						p = getclientaddr(client[i].fd);
						if (p != NULL)
							syslog(LOG_WARNING,
							    "Authtorization error for: %s", p);
						close_conn(client, i);
					}
					continue;
				}
				if (client[i].state < AUTHTORIZED) {
#ifdef DEBUG
					syslog(LOG_DEBUG, "client state: %d\n",
					       client[i].state);
#endif
					get_err(NAUTH_ERR, &client[i]);
/*
			fcntl(client[i].fd,F_SETFL,O_NONBLOCK);
			write(client[i].fd,client[i].buf,
						client[i].bufload);
			client[i].wp = client[i].buf;
			client[i].bufload = 0;
*/
					tfds.fd = client[i].fd;
					tfds.events = POLLOUT;
					if (poll(&tfds, 1, 0) > 0) {
						write(client[i].fd,
						    client[i].buf,
						    client[i].bufload);
						client[i].wp = client[i].buf;
						client[i].bufload = 0;
					}
					close_conn(client, i);
					continue;
				}
				if (c->cmdcode == ERROR_CMD) {
					get_err(UNKNOWN_ERR, &client[i]);
					client[i].nstate = client[i].state;
					client[i].state = WRITE_DATA;
					continue;
				}
				client[i].timeout = READ_TMOUT;
				switch (c->cmdcode) {
				case STAT_CMD:
				    if (statsock >= 0) {
					    get_err(LOCK_ERR, &client[i]);
					    client[i].nstate = client[i].state;
					    client[i].state = WRITE_DATA;
					    break;
				    }
				    statsock = client[i].fd;
				    bhp = bucket_prn;
				    blhp = bucket_prn_len;
				    bucket_prn = bucket_pass;
				    bucket_prn_len = bucket_pass_len;
				    bucket_pass = bhp;
				    bucket_pass_len = blhp;
				    write_time_to_buf(pass_time, time(NULL),
						      &client[i]);
				    pass_time = time(NULL);
				    client[i].nstate = client[i].state;
				    client[i].state = SEND_IP_STAT;
				    break;
				case LOAD_CMD:
				    write_loadstat_to_buf(&client[i]);
				    client[i].nstate = CLOSE_CONN;
				    client[i].state = WRITE_DATA;
				    break;
				case PORT_CMD:
				    if ((*cmdbuf) == NULL ||
					!strncasecmp("tcp", cmdbuf, 4)) {
					    write_portstat_to_buf(IPPROTO_TCP,
						      &client[i]);
				    }
				    if ((*cmdbuf) == NULL ||
					!strncasecmp("udp", cmdbuf, 4)) {
					    write_portstat_to_buf(IPPROTO_UDP,
						      &client[i]);
				    }
				    client[i].nstate = CLOSE_CONN;
				    client[i].state = WRITE_DATA;
				    break;
				case PROTO_CMD:
				    write_protostat_to_buf(&client[i]);
				    client[i].nstate = CLOSE_CONN;
				    client[i].state = WRITE_DATA;
				    break;
				case NOP_CMD:
				    break;
				case HELP_CMD:
				    cmd_help(&client[i]);
				    client[i].nstate = CLOSE_CONN;
				    client[i].state = WRITE_DATA;
				    break;
				case QUIT_CMD:
				    p = getclientaddr(client[i].fd);
				    if (p != NULL)
					    syslog(LOG_INFO,
						"%s quit", p);
				    close_conn(client, i);
				    break;
				case STOP_CMD:
				    p = getclientaddr(client[i].fd);
				    if (p != NULL)
					    syslog(LOG_WARNING,
						   "STOP command from: %s", p);
				    stop();
				    break;
#ifdef	DEBUG
				case VERSION_CMD:

				    /*
				     * remove or make
				     * get_version()
				     */
				    strncpy(client[i].buf, ipstatd_ver,
					    client[i].bufsize);
				    strncat(client[i].buf, "\n",
					    client[i].bufsize - strlen(client[i].buf));
				    strncat(client[i].buf, net_ver,
					    client[i].bufsize - strlen(client[i].buf));
				    strncat(client[i].buf, "\n",
					    client[i].bufsize - strlen(client[i].buf));
				    client[i].bufload = strlen(client[i].buf);
				    client[i].nstate = CLOSE_CONN;
				    client[i].state = WRITE_DATA;
				    break;
				case DEBUG_CMD:
				    print_debug(&client[i]);
				    break;
#endif
				}
				continue;
			}
		}
	    }
	}
	return (0);
}

#ifdef	DEBUG
int
print_debug(struct conn *client)
{
	syslog(LOG_DEBUG, "fd: %d\n", client->fd);
	syslog(LOG_DEBUG, "nstate: %d\n", client->nstate);
	syslog(LOG_DEBUG, "state: %d\n", client->state);
	syslog(LOG_DEBUG, "rw_fl: %d\n", client->rw_fl);
	syslog(LOG_DEBUG, "bufload: %d\n", client->bufload);
	syslog(LOG_DEBUG, "bufsize: %d\n", client->bufsize);
	syslog(LOG_DEBUG, "bn: %d\n", client->bn);
	syslog(LOG_DEBUG, "bi: %d\n", client->bi);
	return (0);
}
#endif

int
write_data_to_sock(struct conn *client)
{
	int wb;

	wb = write(client->fd, client->wp, client->bufload);
	if (wb == -1) {
		syslog(LOG_ERR, "write: %m");
		return (-1);
	}
	client->wp += wb;
	client->bufload -= wb;
	return (0);
}

int
cmd_help(struct conn *client)
{
	int len, size = client->bufsize;
	struct cmd *c;

	client->wp = client->buf;
	for (c = cmdtab; (c->cmdname != NULL) && (size > 0); c++) {
		len = snprintf(client->wp, size, "%s %s\n", c->cmdname,
		    c->cmdhelp);
		if (len >= size)
			return (1);
		client->wp += len;
		size -= len;
	}
	client->bufload = client->bufsize - size;
	client->wp = client->buf;
	return (client->bufsize);
}

int
close_conn(struct conn *client, int k)
{
	int i;

	free(client[k].chal);
	free(client[k].buf);
	free(client[k].rbuf);
	if (statsock == client[k].fd)
		statsock = -1;
	if (maxsock == client[k].fd) {
		maxsock = 0;
		for (i = 0; (i < MAX_ACT_CONN); i++)
			if (i != k)
				maxsock = MAX(maxsock, client[i].fd);
	}
	if (close(client[k].fd) == -1)
		syslog(LOG_ERR, "%s: close: %m, fd = %d",
		    __func__, client[k].fd);

	client[k].fd = -1;
	nos--;

	return (0);
}

int
get_err(int errnum, struct conn *client)
{
	struct err *e;
	int len, size;

	for (e = errtab; e->errnum != NULL; e++)
		if (errnum == e->errnum)
			break;
	size = client->bufsize - client->bufload;
	len = snprintf(client->buf + client->bufload, size,
		"%d - %s\n", errnum, e->errdesc);
	if (len >= size)
		return (1);
	client->bufload += len;
	return (0);
}


__dead void
stop(void)
{
	int i;

	if (close(lisn_fds.fd) == -1)
		syslog(LOG_ERR, "%s: close: %m", __func__);

	for (i = 0; i < MAX_ACT_CONN; i++) {
		if (client[i].fd >= 0) {
			get_err(STOP_ERR, &client[i]);
			fcntl(client[i].fd, F_SETFL, O_NONBLOCK);
			write(client[i].fd, client[i].buf, client[i].bufload);
			close_conn(client, i);
		}
	}

	if (cap->close)
		cap->close();

	/* flush stat to disk */
	syslog(LOG_INFO, "Exiting...\n");

	exit(0);
}

char*
getclientaddr(int fd)
{
	int err;
	struct sockaddr_in sock_client;
	int addrlen = sizeof(sock_client);

	err = getpeername(fd, (struct sockaddr *) & sock_client, &addrlen);
	if (err == -1) {
		syslog(LOG_ERR, "getpeername: %m");
		return (NULL);
	}
	return (inet_ntoa(sock_client.sin_addr));
}
