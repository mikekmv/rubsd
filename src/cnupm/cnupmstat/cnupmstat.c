/*	$RuOBSD: cnupmstat.c,v 1.7 2004/01/14 05:26:51 form Exp $	*/

/*
 * Copyright (c) 2003 Oleg Safiullin <form@pdp-11.org.ru>
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

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cnupm.h"
#include "collect.h"

#ifdef __FreeBSD__
#define __dead		__dead2
#endif

static struct passwd *pw;
static char *cnupm_user = CNUPM_USER;
static char cnupm_delim = ' ';
static int Bflag;
static int Eflag;
static int Fflag;
static int nflag;
static int Nflag;
static int Pflag;
static sa_family_t family;
static int proto = -1;

int main(int, char **);
__dead static void usage(void);
static int print_dumpfile(const char *);

int
main(int argc, char **argv)
{
	int ch, retval = 0;

	while ((ch = getopt(argc, argv, "Bd:Ef:FnNp:Pu:V")) != -1)
		switch (ch) {
		case 'B':
			Bflag = 1;
			break;
		case 'd':
			cnupm_delim = *optarg;
			break;
		case 'E':
			Eflag = 1;
			break;
		case 'f':
			if (!strcmp(optarg, "inet")) {
				family = AF_INET;
				break;
			}
			if (!strcmp(optarg, "inet6")) {
				family = AF_INET6;
				break;
			}
			errx(1, "%s: Address family not supported", optarg);
			/* NOTREACHED */
		case 'F':
			Fflag = 1;
			break;
		case 'n':
			nflag = 1;
			break;
		case 'N':
			Nflag = 1;
			break;
		case 'p':
			{
				struct protoent *pe;
				unsigned long ulval;
				char *ep;

				if ((pe = getprotobyname(optarg)) != NULL) {
					proto = pe->p_proto;
					break;
				}
				ulval = strtoul(optarg, &ep, 0);
				if (optarg[0] == '\0' || *ep != '\0' ||
				    ulval > UCHAR_MAX)
					errx(1, "%s: Protocol not supported",
					    optarg);
				proto = (int)ulval;
			}
			break;
		case 'P':
			Pflag = 1;
			break;
		case 'u':
			cnupm_user = optarg;
			break;
		case 'V':
			(void)fprintf(stderr, "cnupmstat v%u.%u\n",
			    CNUPM_VERSION_MAJOR, CNUPM_VERSION_MINOR);
			return (0);
		default:
			usage();
			/* NOTREACHED */
		}
	argv += optind;
	argc -= optind;
	if (argc == 0)
		usage();

	if (!Fflag) {
		if ((pw = getpwnam(cnupm_user)) == NULL)
			errx(1, "No passwd entry for %s", cnupm_user);
		if (pw->pw_dir == NULL || pw->pw_dir[0] == '\0')
			errx(1, "No home directory for %s", cnupm_user);
	}

	for (ch = 0; ch < argc; ch++)
		retval |= print_dumpfile(argv[ch]);

	return (retval);
}

static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-BEFnNPV] [-d delim ] [-f family] [-p protocol] "
	    "[-u user]\n                 interface [...]\n", __progname);
	exit(1);
}

static int
print_dumpfile(const char *interface)
{
	char file[MAXPATHLEN];
	struct coll_header ch;
	struct coll_traffic ct;
	ssize_t nbytes;
	int fd, i;

	if (Fflag)
		fd = open(interface, O_RDONLY);
	else {
		(void)snprintf(file, sizeof(file), "%s/" CNUPM_DUMPFILE,
		    pw->pw_dir, interface);
		fd = open(file, O_RDONLY);
	}
	if (fd < 0) {
		warn("open: %s", Fflag ? interface : file);
		return (1);
	}

	while ((nbytes = read(fd, &ch, sizeof(ch))) == sizeof(ch)) {
		char start[20], stop[20];

		ch.ch_flags = ntohl(ch.ch_flags);
		ch.ch_start = ntohl(ch.ch_start);
		ch.ch_stop = ntohl(ch.ch_stop);
		ch.ch_count = ntohl(ch.ch_count);
		if (CNUPM_MAJOR(ch.ch_version) > CNUPM_VERSION_MAJOR) {
			warnx("%s: Incompatible file format%s",
			    Fflag ? " for" : "",
			    Fflag ? interface : file);
			(void)close(fd);
			return (1);
		}
		if (!Bflag)
			(void)strftime(start, sizeof(start), "%Y-%m-%d %H:%M:%S",
			    localtime(&ch.ch_start));
		if (!Eflag)
			(void)strftime(stop, sizeof(stop), "%Y-%m-%d %H:%M:%S",
			    localtime(&ch.ch_stop));

		for (i = 0; i < ch.ch_count; i++) {
			char addr[INET6_ADDRSTRLEN];
			struct protoent *pe;

			if ((nbytes = read(fd, &ct, sizeof(ct))) != sizeof(ct))
				break;
			if (family && ct.ct_family != family)
				continue;
			if (proto >= 0 && ct.ct_proto != proto)
				continue;
			ct.ct_bytes = betoh64(ct.ct_bytes);
			if (!Bflag)
				(void)printf("%s%c", start, cnupm_delim);
			if (!Eflag)
				(void)printf("%s%c", stop, cnupm_delim);
			(void)printf("%s", inet_ntop(ct.ct_family, &ct.ct_src,
			    addr, sizeof(addr)));
			if (!Pflag && (ct.ct_proto == IPPROTO_TCP ||
			    ct.ct_proto == IPPROTO_UDP) && ct.ct_sport)
				(void)printf("%c%u", ct.ct_family == AF_INET ?
				    ':' : '.', ntohs(ct.ct_sport));
			(void)printf("%c%s", cnupm_delim,
			    inet_ntop(ct.ct_family, &ct.ct_dst, addr,
			    sizeof(addr)));
			if (!Pflag && (ct.ct_proto == IPPROTO_TCP ||
			    ct.ct_proto == IPPROTO_UDP) && ct.ct_dport)
				(void)printf("%c%u", ct.ct_family == AF_INET ?
				    ':' : '.', ntohs(ct.ct_dport));
			if (!Nflag) {
				if (nflag || ((pe =
				    getprotobynumber(ct.ct_proto))) == NULL)
					(void)printf("%c%u", cnupm_delim,
					    ct.ct_proto);
				else
					(void)printf("%c%s", cnupm_delim,
					    pe->p_name);
			}
			(void)printf("%c%llu\n", cnupm_delim, ct.ct_bytes);
		}
		if (nbytes != sizeof(ct))
			break;
	}
	if (nbytes != 0) {
		if (nbytes < 0) {
			warn("read: %s", Fflag ? interface : file);
		} else
			warnx("%s: File data corrupt", Fflag ?
			    interface : file);
		(void)close(fd);
		return (1);
	}
	(void)close(fd);

	return (0);
}
