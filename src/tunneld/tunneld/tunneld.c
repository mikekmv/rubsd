/*	$RuOBSD: tunneld.c,v 1.9 2001/11/20 03:12:20 form Exp $	*/

/*
 * Copyright (c) 2001 Oleg Safiullin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#ifdef HAS_PIDFILE
#include <util.h>
#else
#include "pidfile.h"
#endif

#ifdef NAT
#include <alias.h>
#endif

#define TUN_MAX		255

#ifndef IPPROTO_IPIP
#define IPPROTO_IPIP	4
#endif

#ifndef IPPROTO_IPENCAP
#define IPPROTO_IPENCAP	94
#endif

int main __P((int, char **));
void usage __P((void));
int protocol __P((char *));
void setsockaddr __P((char *, struct sockaddr_in *, int));
int opentun __P((char *));
void ifconfig __P((char *, char *, char *));
void ifunconfig __P((void));
void sighandler __P((int));
void mkpidfile __P((void));
#ifdef CRYPT
void encryptpkt __P((struct ip *));
#define decryptpkt encryptpkt
#endif

extern char *__progname;
extern int h_errno;

#ifdef CRYPT
static int eflag = 0;
#endif
#ifdef NAT
static u_int nat_mode = 0;
static int nflag = 0;
#endif
static int vflag = 0;
static int proto = IPPROTO_IPENCAP;
static struct sockaddr_in ssa, dsa;
static char *tun = NULL;

sigset_t allsigs;

int
main(argc, argv)
	int argc;
	char **argv;
{
	int opt, s, t;
	struct pollfd fds[2];

	setsockaddr("0.0.0.0", &ssa, 0);
#ifdef	CRYPT
#define CRYPT_FLAGS "E"
#else	/* !CRYPT */
#define CRYPT_FLAGS
#endif	/* CRYPT */
#ifdef	NAT
#define NAT_FLAGS "dmNsu"
#else	/* !NAT */
#define NAT_FLAGS
#endif	/* NAT */
#define GETOPT_FLAGS CRYPT_FLAGS NAT_FLAGS
	while ((opt = getopt(argc, argv, "p:b:t:v" GETOPT_FLAGS)) != -1) {
		switch (opt) {
		case 'b':
			setsockaddr(optarg, &ssa, 0);
			break;
		case 'p':
			proto = protocol(optarg);
			break;
		case 't':
			tun = optarg;
			break;
		case 'v':
			vflag = 1;
			break;
#ifdef	CRYPT
		case 'E':
			eflag = 1;
			break;
#endif	/* CRYPT */
#ifdef	NAT
		case 'd':
			nat_mode |= PKT_ALIAS_DENY_INCOMING;
			break;
		case 'm':
			nat_mode |= PKT_ALIAS_SAME_PORTS;
			break;
		case 's':
			nat_mode |= PKT_ALIAS_USE_SOCKETS;
			break;
		case 'u':
			nat_mode |= PKT_ALIAS_UNREGISTERED_ONLY;
			break;
		case 'N':
			nflag = 1;
			break;
#endif	/* NAT */
		default:
			usage();
			break;
		}
	}
	argv += optind;
	if ((argc -= optind) != 1 && argc != 3 && argc != 4)
		usage();

#ifdef	NAT
	if (nat_mode)
		nflag = 1;
	if (nflag && argc < 3)
		errx(1, "can't use -N without destination address");
#endif	/* NAT */

	setsockaddr(argv[0], &dsa, 0);
	t = opentun(tun);
	if ((s = socket(PF_INET, SOCK_RAW, proto)) < 0)
		err(1, "socket");

	if (bind(s, (struct sockaddr *)&ssa, sizeof(ssa)) < 0)
		err(1, "bind");
	if (connect(s, (struct sockaddr *)&dsa, sizeof(dsa)) < 0)
		err(1, "connect");

	if (argc > 1)
		ifconfig(argv[1], argv[2],
		    (argc > 3 ? argv[3] : "255.255.255.255"));

	openlog(__progname, LOG_PID | (vflag ? LOG_PERROR : 0), LOG_DAEMON);
	sigfillset(&allsigs);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGTERM, sighandler);

	if (!vflag) {
		if (daemon(0, 0) < 0)
			err(1, "daemon");
		mkpidfile();
	}

	fds[0].fd = t;
	fds[1].fd = s;
	fds[0].events = fds[1].events = POLLIN;
	for (;;) {
		u_char buf[IP_MAXPACKET];
		struct ip *ip = (void *)buf;

		if (poll(fds, 2, -1) < 0) {
			syslog(LOG_ERR, "poll: %m");
			sighandler(-1);
		}
		if (fds[0].revents & POLLIN) {
			size_t nb;

			if ((nb = read(t, buf, sizeof(buf))) < 0)
				syslog(LOG_WARNING, "read: %m");
			else {
#ifdef	NAT
				if (nflag)
#ifdef	__OpenBSD__
					PacketAliasOut((char *)buf +
					    sizeof(u_int32_t),
					    sizeof(buf) - sizeof(u_int32_t));
#else	/* !__OpenBSD__ */
					PacketAliasOut((char *)buf,
					    sizeof(buf));
#endif	/* __OpenBSD__ */
#endif	/* NAT */
#ifdef	CRYPT
				if (eflag)
#ifdef	__OpenBSD__
					encryptpkt((void *)(buf +
					    sizeof(u_int32_t)));
#else	/* !__OpenBSD__ */
					encryptpkt((void *)(buf));
#endif	/* __OpenBSD__ */
#endif	/* CRYPT */
#ifdef	__OpenBSD__
				if (send(s, buf + sizeof(u_int32_t),
				    nb - sizeof(u_int32_t), 0) < 0 &&
				    errno != ENOBUFS)
#else	/* !__OpenBSD__ */
				if (send(s, buf, nb, 0) < 0 &&
				    errno != ENOBUFS)
#endif	/* __OpenBSD__ */
					syslog(LOG_WARNING, "send: %m");
			}
		}
		if (fds[1].revents & POLLIN) {
			size_t nb;
			u_int ipoff;

			if ((nb = recv(s, buf, sizeof(buf), 0)) < 0)
				syslog(LOG_WARNING, "recv: %m");
			else {
				if (ip->ip_src.s_addr == dsa.sin_addr.s_addr) {
#ifdef	__OpenBSD__
					ipoff = (ip->ip_hl << 2) -
					    sizeof(u_int32_t);
#else	/* !__OpenBSD__ */
					ipoff = ip->ip_hl << 2;
#endif	/* __OpenBSD__ */
#ifdef	CRYPT
					if (eflag)
#ifdef	__OpenBSD__
						decryptpkt((void *)
						    (buf + ipoff +
						    sizeof(u_int32_t)));
					*(u_int32_t *)(buf + ipoff) =
#ifdef	TUN_AF_HOSTORDER
					    ssa.sin_family;
#else	/* !TUN_AF_HOSTORDER */
					    htonl(ssa.sin_family);
#endif	/* TUN_AF_HOSTORDER */
#else	/* !__OpenBSD__ */
						decryptpkt((void *)
						    (buf + ipoff));
#endif	/* __OpenBSD__ */
#endif	/* CRYPT */
#ifdef	NAT
					if (nflag)
#ifdef	__OpenBSD__
						PacketAliasIn((char *)buf +
						    ipoff + sizeof(u_int32_t),
						    sizeof(buf) - ipoff -
						    sizeof(u_int32_t));
#else	/* !__OpenBSD__ */
						PacketAliasIn((char *)buf +
						    ipoff,
						    sizeof(buf) - ipoff);
#endif	/* __OpenBSD__ */
#endif	/* NAT */
					if (write(t, buf + ipoff,
					    nb - ipoff) < 0)
						syslog(LOG_WARNING,
						    "write: %m");
				}
			}
		}
	}

	return (0);
}

#ifdef CRYPT
#ifdef NAT
#define USAGE_FLAGS "-dEmNsuv"
#else	/* !NAT */
#define USAGE_FLAGS "-Ev"
#endif	/* NAT */
#else	/* !CRYPT */
#ifdef NAT
#define USAGE_FLAGS "-dmNsuv"
#else	/* !NAT */
#define USAGE_FLAGS "-v"
#endif	/* NAT */
#endif	/* CRYPT */

void
usage(void)
{
	fprintf(stderr,
	    "usage: %s [" USAGE_FLAGS "] "
	    "[-b src] [-p proto] [-t tun] dst [local remote [mask]]\n",
	    __progname);
	exit(1);
}

int
protocol(p)
	char *p;
{
	struct protoent *pe;
	u_long value;

	if ((pe = getprotobyname(p)) == NULL) {
		char *eptr;

		if ((value = strtoul(p, &eptr, 10)) >= IPPROTO_MAX || *eptr)
			errx(1, "%s: illegal protocol", p);
	} else
		value = pe->p_proto;
	if (value != IPPROTO_IPIP && value != IPPROTO_IPENCAP)
		errx(1, "%s: unsupported protocol", p);

	return (value);
}

void
setsockaddr(a, sa, mask)
	char *a;
	struct sockaddr_in *sa;
	int mask;
{
	bzero(sa, sizeof(*sa));
	sa->sin_family = AF_INET;
	sa->sin_len = sizeof(*sa);
	if (inet_pton(AF_INET, a, &sa->sin_addr) != 1) {
		struct hostent *he;

		if (mask)
			errx(1, "%s: invalid netmask", a);
		if ((he = gethostbyname(a)) == NULL)
			errx(1, "gethostbyname(\"%s\"): %s", a,
			    hstrerror(h_errno));
		sa->sin_family = he->h_addrtype;
		bcopy(he->h_addr, &sa->sin_addr, he->h_length);
	}
}

int
opentun(t)
	char *t;
{
	static char dev[12];
	int fd = -1;

	if (tun != NULL) {
		char *eptr;

		if (strncmp(t, "tun", 3) ||
		    (strtoul(&t[3], &eptr, 10) > TUN_MAX || *eptr))
			errx(1, "%s: illegal interface", t);
		snprintf(dev, sizeof(dev), "/dev/%s", t);
		fd = open(dev, O_RDWR | O_NONBLOCK);
	} else {
		int u;

		for (u = 0; u < TUN_MAX; u++) {
			snprintf(dev, sizeof(dev), "/dev/tun%d", u);
			if ((fd = open(dev, O_RDWR | O_NONBLOCK)) < 0 &&
			    errno == EBUSY)
				continue;
			break;
		}
		if (vflag && fd >= 0)
			warnx("using %s", dev);
	}
	if (fd < 0)
		err(1, "open(\"%s\")", dev);
	tun = &dev[5];
	return (fd);
}

void
ifconfig(local, remote, mask)
	char *local;
	char *remote;
	char *mask;
{
	int s;
	struct ifaliasreq ifra;
	struct sockaddr_in *addr = (void *)&ifra.ifra_addr;

	strncpy(ifra.ifra_name, tun, sizeof(ifra.ifra_name));
	setsockaddr(local, (void *)&ifra.ifra_addr, 0);
	setsockaddr(remote, (void *)&ifra.ifra_broadaddr, 0);
	setsockaddr(mask, (void *)&ifra.ifra_mask, 1);

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err(1, "ifconfig socket");
	if (ioctl(s, SIOCAIFADDR, &ifra) < 0)
		err(1, "SIOCAIFADDR");
	close(s);
#ifdef NAT
	if (nflag) {
		PacketAliasInit();
		PacketAliasSetMode(0,
		    PKT_ALIAS_SAME_PORTS | PKT_ALIAS_USE_SOCKETS |
		    PKT_ALIAS_UNREGISTERED_ONLY);
		PacketAliasSetAddress(addr->sin_addr);
		PacketAliasSetMode(nat_mode, nat_mode);
	}
#endif	/* NAT */
}

void
ifunconfig(void)
{
	int s;
	struct ifreq ifrq;

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err(1, "ifunconfig socket");
	strncpy(ifrq.ifr_name, tun, sizeof(ifrq.ifr_name));
	if (ioctl(s, SIOCDIFADDR, &ifrq) < 0)
		err(1, "SIOCDIFADDR");
	close(s);
#ifdef NAT
	if (nflag)
		PacketAliasUninit();
#endif
}

void
sighandler(signo)
	int signo;
{
	sigprocmask(SIG_BLOCK, &allsigs, NULL);
	if (signo != -1)
		syslog(LOG_INFO, "exiting on signal %d", signo);
	ifunconfig();
	exit(1);
}

void
mkpidfile()
{
	char *file;
	int len = strlen(__progname) + 7;

	if ((file = malloc(len)) == NULL)
		syslog(LOG_WARNING, "mkpidfile: not enough memory");
	else {
		snprintf(file, len, "%s-%s", __progname, tun);
		if (pidfile(file) < 0)
			syslog(LOG_WARNING, "pidfile");
		free(file);
	}
}

#ifdef CRYPT
#define XOR_MASK	0xa5

void
encryptpkt(ip)
	struct ip *ip;
{
	u_short len = ntohs(ip->ip_len);
	u_char *p = (u_char *)ip + (ip->ip_hl << 2);

	while (len-- > 0)
		*p++ ^= XOR_MASK;
}
#endif	/* CRYPT */
