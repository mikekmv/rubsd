/*	$RuOBSD$	*/

/*
 * Copyright (c) 2003 Oleg Safiullin
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
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "collect.h"
#include "trafd.h"

static int family = -1;
static int proto = -1;
static int exit_status;
static struct passwd *pw;

int main(int, char **);
__dead static void usage(void);
static void print_stats(const char *);

int main(int argc, char **argv)
{
	int i;

	while ((i = getopt(argc, argv, "f:p:")) != -1)
		switch (i) {
		case 'f':
			if (!strcmp(optarg, "inet")) {
				family = AF_INET;
				break;
			}
#ifdef INET6
			if (!strcmp(optarg, "inet6")) {
				family = AF_INET6;
				break;
			}
			errx(1, "%s: %s", optarg, strerror(EAFNOSUPPORT));
			/* NOTREACHED */
#endif	/* INET6 */
		case 'p':
			{
				struct protoent *pe;
				unsigned long u;
				char *ep;

				if ((pe = getprotobyname(optarg)) != NULL) {
					proto = pe->p_proto;
					break;
				}
				errno = 0;
				u = strtoul(optarg, &ep, 0);
				if (*ep != '\0' || errno == ERANGE ||
				    u > UCHAR_MAX)
					errx(1, "%s: %s", optarg,
					    strerror(EPROTONOSUPPORT));
				proto = u;
			}
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argv += optind;
	if ((argc -= optind) == 0)
		usage();

	if ((pw = getpwnam(TRAFD_USER)) == NULL)
		errx(1, "%s: Unknown user", TRAFD_USER);
	if (pw->pw_dir == NULL || pw->pw_dir[0] == '\0')
		errx(1, "Invalid home directory for %s", pw->pw_name);

	for (i = 0; i < argc; i++)
		print_stats(argv[i]);

	return (exit_status);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-f family] [-p protocol] interface ...\n",
	    __progname);
	exit(1);
}

static void
print_stats(const char *device)
{
	char file[MAXPATHLEN];
	struct ic_header ih;
	struct ic_traffic it;
	int fd;

	(void)snprintf(file, sizeof(file), "%s/traffic.%s", pw->pw_dir,
	    device);
	if ((fd = open(file, O_RDONLY)) < 0) {
		exit_status = 1;
		warn("open: %s", file);
		return;
	}

	while (read(fd, &ih, sizeof(ih)) == sizeof(ih)) {
#ifdef INET6
		char addr[INET6_ADDRSTRLEN];
#else
		char addr[INET_ADDRSTRLEN];
#endif
		char ts[20];
		struct protoent *pe;
		int i;

		(void)strftime(ts, sizeof(ts), "%Y/%m/%d %H:%M:%S",
		    localtime(&ih.ich_time));
		(void)printf("\n%s (%u)\n", ts, ih.ich_length);

		for (i = 0; i < ih.ich_length; i++) {
			if (read(fd, &it, sizeof(it)) != sizeof(it))
				break;
			if (family >= 0 && it.ict_family != family)
				continue;
			if (proto >= 0 && it.ict_proto != proto)
				continue;
			printf("%llu", it.ict_bytes);
			if ((pe = getprotobynumber(it.ict_proto)) == NULL)
				printf(" %u", it.ict_proto);
			else
				printf(" %s", pe->p_name);
			printf(" %s", inet_ntop(it.ict_family, &it.ict_src,
			    addr, sizeof(addr)));
#ifndef NOPORTS
			if (it.ict_proto == IPPROTO_TCP ||
			    it.ict_proto == IPPROTO_UDP)
#ifdef INET6
				printf("%s%u",
				    (it.ict_family == AF_INET ? ":" : "."),
				    ntohs(it.ict_sport));
#else	/* !INET6 */
				printf(":%u", ntohs(it.ict_sport));
#endif	/* INET6 */
#endif	/* NOPORTS */
			printf(" %s", inet_ntop(it.ict_family, &it.ict_dst,
			    addr, sizeof(addr)));
#ifndef NOPORTS
			if (it.ict_proto == IPPROTO_TCP ||
			    it.ict_proto == IPPROTO_UDP)
#ifdef INET6
				printf("%s%u",
				    (it.ict_family == AF_INET ? ":" : "."),
				    ntohs(it.ict_dport));
#else	/* !INET6 */
				printf(":%u", ntohs(it.ict_dport));
#endif	/* INET6 */
#endif	/* NOPORTS */
			printf("\n");
		}
	}

	(void)close(fd);
}
