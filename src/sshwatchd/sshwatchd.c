/*	$RuOBSD$	*/

/*
 * Copyright (c) 2005-2006 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <sys/time.h>
#include <sys/tree.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/pfvar.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>


#define DEF_LOGFILE		"/var/log/authlog"
#define DEF_MAXFAILS		8
#define MIN_MAXFAILS		1
#define MAX_MAXFAILS		100
#define DEF_SECONDS		5
#define MIN_SECONDS		1
#define MAX_SECONDS		86400
#define RETRY_COUNT		10
#define BUFFER_SIZE		4096
#define PF_TABLE		"blocked"
#define PF_DEVICE		"/dev/pf"
#define DEF_FACILITY		"auth"


struct failure {
	time_t			f_time;
	in_addr_t		f_addr;
	int			f_count;
	RB_ENTRY(failure)	f_entry;
};


RB_HEAD(failure_tree, failure) failures = RB_INITIALIZER(&failures);

static char *logfile = DEF_LOGFILE;
static char *pf_table = PF_TABLE;
static char *facility = DEF_FACILITY;
static int maxfails = DEF_MAXFAILS;
static int seconds = DEF_SECONDS;
static int killstates;
static int debug;
static int critical;
static int pf_fd;


int main(int, char *const *);
__dead static void usage(void);


static __inline int
failure_compare(struct failure *a, struct failure *b)
{
	if (a->f_addr < b->f_addr)
		return (-1);
	if (a->f_addr > b->f_addr)
		return (1);
	return (0);
}

RB_PROTOTYPE(failure_tree, failure, f_entry, failure_compare)
RB_GENERATE(failure_tree, failure, f_entry, failure_compare)


static void
block(in_addr_t addr)
{
	char aaddr[INET_ADDRSTRLEN];
	struct pfioc_table io;
	struct pfr_addr pfra;
	struct pfioc_state_kill psk;

	bzero(&pfra, sizeof(pfra));
	pfra.pfra_ip4addr.s_addr = addr;
	pfra.pfra_af = AF_INET;
	pfra.pfra_net = 32;
	bzero(&io, sizeof(io));
	(void)strlcpy(io.pfrio_table.pfrt_name, pf_table,
	    sizeof(io.pfrio_table.pfrt_name));
	io.pfrio_buffer = &pfra;
	io.pfrio_size = 1;
	io.pfrio_esize = sizeof(pfra);
	if (ioctl(pf_fd, DIOCRADDADDRS, &io) < 0) {
		syslog(LOG_ERR, "DIOCRADDADDRS: %m");
		return;
	}

	(void)inet_ntop(AF_INET, &addr, aaddr, sizeof(aaddr));
	syslog(LOG_NOTICE, "blocked address %s", aaddr);

	if (killstates) {
		bzero(&psk, sizeof(psk));
		psk.psk_af = AF_INET;
		psk.psk_src.addr.v.a.addr.v4.s_addr = addr;
		psk.psk_src.addr.v.a.mask.v4.s_addr = INADDR_BROADCAST;
		if (ioctl(pf_fd, DIOCKILLSTATES, &psk) < 0)
			syslog(LOG_ERR, "DIOCKILLSTATES: %m");
	}
}

static void
parse(char *s)
{
	struct failure *f, *t;
	in_addr_t addr;
	char *cp, *p;

	if ((cp = strstr(s, "Failed password for ")) != NULL &&
	    (cp = strstr(cp, "from")) != NULL &&
	    (cp = strchr(cp, ' ')) != NULL &&
	    (p = strchr(++cp, ' ')) != NULL) {
		*p = '\0';

		if (inet_pton(AF_INET, cp, &addr) != 1)
			return;

		if (maxfails == 1) {
			block(addr);
			return;
		}

		if ((f = malloc(sizeof(*f))) != NULL) {
			f->f_time = time(NULL);
			f->f_addr = addr;
			f->f_count = 1;
			critical++;
			if ((t = RB_INSERT(failure_tree, &failures,
			    f)) != NULL) {
				free(f);
				if (++t->f_count == maxfails) {
					block(t->f_addr);
					(void)RB_REMOVE(failure_tree,
					    &failures, t);
					free(t);
				}
			}
			--critical;
		} else
			syslog(LOG_ERR, "malloc: %m");
	}
}

static int
logfac(const char *f)
{
	if (strcasecmp(f, "auth") == 0)
		return (LOG_AUTH);
	else if (strcasecmp(f, "authpriv") == 0)
		return (LOG_AUTHPRIV);
	else if (strcasecmp(f, "daemon") == 0)
		return (LOG_DAEMON);
	else if (strcasecmp(f, "local0") == 0)
		return (LOG_LOCAL0);
	else if (strcasecmp(f, "local1") == 0)
		return (LOG_LOCAL1);
	else if (strcasecmp(f, "local2") == 0)
		return (LOG_LOCAL2);
	else if (strcasecmp(f, "local3") == 0)
		return (LOG_LOCAL3);
	else if (strcasecmp(f, "local4") == 0)
		return (LOG_LOCAL4);
	else if (strcasecmp(f, "local5") == 0)
		return (LOG_LOCAL5);
	else if (strcasecmp(f, "local6") == 0)
		return (LOG_LOCAL6);
	else if (strcasecmp(f, "local7") == 0)
		return (LOG_LOCAL7);
	return (-1);
}

static void
sigalrm(int signo)
{

	struct failure *f, *t;
	time_t tm;

	if (!critical) {
		tm = time(NULL);
		RB_FOREACH(f, failure_tree, &failures) {
			if (tm - f->f_time > seconds) {
				t = RB_NEXT(failure_tree, &failures, f);
				RB_REMOVE(failure_tree, &failures, f);
				free(f);
				if ((f = t) == NULL)
					break;
			}
		}
	}
}

int
main(int argc, char *const argv[])
{
	char buf[BUFFER_SIZE];
	struct pfr_table pfrt;
	struct pfioc_table io;
	struct kevent kev;
	struct itimerval it;
	const char *errstr;
	ssize_t nb, off = 0, len;
	int ch, kq, fd;
	char *cp, *p;

	while ((ch = getopt(argc, argv, "df:kl:m:s:t:")) != -1)
		switch (ch) {
		case 'd':
			debug++;
			break;
		case 'f':
			logfile = optarg;
			break;
		case 'k':
			killstates++;
			break;
		case 'l':
			facility = optarg;
			break;
		case 'm':
			maxfails = strtonum(optarg, MIN_MAXFAILS,
			    MAX_MAXFAILS, &errstr);
			if (errstr != NULL)
				errx(EX_CONFIG, "-m %s: %s", optarg, errstr);
			break;
		case 's':
			seconds = strtonum(optarg, MIN_SECONDS,
			    MAX_SECONDS, &errstr);
			if (errstr != NULL)
				errx(EX_CONFIG, "-s %s: %s", optarg, errstr);
			break;
		case 't':
			pf_table = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	if (argc != 0)
		usage();

	if ((pf_fd = open(PF_DEVICE, O_RDWR)) < 0)
		err(EX_UNAVAILABLE, "open: %s", PF_DEVICE);

	bzero(&pfrt, sizeof(pfrt));
	strlcpy(pfrt.pfrt_name, pf_table, sizeof(pfrt.pfrt_name));
	bzero(&io, sizeof(io));
	io.pfrio_buffer = &pfrt;
	io.pfrio_size = 1;
	io.pfrio_esize = sizeof(pfrt);
	pfrt.pfrt_flags = PFR_TFLAG_PERSIST;
	if (ioctl(pf_fd, DIOCRADDTABLES, &io) < 0)
		err(EX_UNAVAILABLE, "DIOCRADDTABLES");

	if (maxfails > 1) {
		if (signal(SIGALRM, sigalrm) == SIG_ERR)
			err(EX_UNAVAILABLE, "signal");
		it.it_interval.tv_sec = it.it_value.tv_sec = seconds;
		it.it_interval.tv_usec = it.it_value.tv_usec = 500000;
		if (setitimer(ITIMER_REAL, &it, NULL) < 0)
			err(EX_UNAVAILABLE, "setitimer");
	}

	if ((ch = logfac(facility)) < 0)
		errx(EX_CONFIG, "Unsupported log facility %s", facility);

	if (!debug && daemon(0, 0) < 0)
		err(EX_OSERR, "daemon");

	if ((kq = kqueue()) < 0) {
		syslog(LOG_ERR, "kqueue: %m");
		return (EX_UNAVAILABLE);
	}

	openlog("sshwatchd", debug ? LOG_PID | LOG_PERROR : LOG_PID, ch);
	(void)setproctitle("watching for %s", logfile);

open:	for (ch = 0; ch < RETRY_COUNT; ch++) {
		if ((fd = open(logfile, O_RDONLY)) < 0) {
			if (errno != ENOENT)
				break;
			(void)sleep(1);
			continue;
		}
		break;
	}

	if (fd < 0) {
		syslog(LOG_ERR, "open: %s: %m", logfile);
		return (EX_UNAVAILABLE);
	}

	EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_DELETE |
	    NOTE_EXTEND | NOTE_TRUNCATE | NOTE_RENAME | NOTE_REVOKE, 0, NULL);

	ch = 1;

seek:	if (lseek(fd, 0, SEEK_END) < 0) {
		syslog(LOG_ERR, "lseek: %s: %m", logfile);
		return (EX_IOERR);
	}

	for (;;) {
		if (kevent(kq, &kev, ch, &kev, 1, NULL) < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "kevent: %m");
			return (EX_UNAVAILABLE);
		}
		ch = 0;

		if (kev.fflags & (NOTE_DELETE | NOTE_REVOKE | NOTE_RENAME))
			break;

		if (kev.fflags & NOTE_TRUNCATE) {
			syslog(LOG_WARNING, "log file truncated");
			goto seek;
		}

		if (kev.fflags & NOTE_EXTEND) {
			if ((nb = read(fd, buf + off,
			    sizeof(buf) - off - 1)) < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_ERR, "read: %s: %m", logfile);
				return (EX_IOERR);
			}

			buf[nb += off] = '\0';
			if ((cp = strchr(p = buf, '\n')) == NULL) {
				if (nb == sizeof(buf) - 1) {
					syslog(LOG_WARNING, "line too long");
					off = 0;
				} else
					off = nb;
				continue;
			}

			do {
				*cp++ = '\0';
				nb -= (len = strlen(p)) + 1;
				parse(p);
				cp = strchr(p = cp, '\n');
			} while (cp != NULL);

			if ((off = nb) != 0)
				bcopy(p, buf, nb);
		}
	}

	(void)close(fd);

	if (kev.fflags & NOTE_REVOKE) {
		syslog(LOG_ERR, "Access to logfile has been revoked");
		return (EX_IOERR);
	}

	syslog(LOG_NOTICE, "%s has been %s", logfile,
	    kev.fflags & NOTE_DELETE ? "deleted" : "renamed");
	goto open;

	/* NOTREACHED */
	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s [-dk] [-f logfile] [-l facility] "
	    "[-m maxfails] [-s seconds] [-t table]\n", __progname);
	exit(EX_USAGE);
}
