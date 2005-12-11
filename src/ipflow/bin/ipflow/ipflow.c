/*	$RuOBSD: ipflow.c,v 1.2 2005/11/14 21:19:38 form Exp $	*/

/*
 * Copyright (c) 2005 Oleg Safiullin <form@pdp-11.org.ru>
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
#include <sys/stat.h>
#include <net/if.h>
#include <net/bpf.h>
#include <net/ipflow.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>


struct link_type {
	u_int		dlt;
	char		*type;
};


int main(int, char *const *);
__dead static void usage(void);
static char *copy_argv(char * const *);
static char *copy_file(const char *);
static char *link_type(u_int);
static void print_if(struct ipflow_ifreq *);


static char *file;
static int cmd;
static int appnd;
static int flush;
static int syncf;
static size_t nflows;


#define CMD_FLUSH		1
#define CMD_FLOWS		2
#define CMD_SHOW		3
#define CMD_LOAD		4
#define CMD_SAVE		5
#define CMD_ADD			6
#define CMD_SET			7
#define CMD_DEL			8
#define CMD_INFO		9
#define CMD_RESET		10
#define CMD_VERSION		11

#define SAVE_FILE_MODE		0644


int
main(int argc, char *const argv[])
{
	struct bpf_program bprog;
	struct ipflow_version iv;
	struct ipflow_ifreq iir;
	struct ipflow_req irq;
	struct ipflow_info ifi;
	int ch, dd, fd = -1;
	char *filter;
	ssize_t n;

	opterr = 0;
	while ((ch = getopt(argc, argv, "afy")) != -1)
		switch (ch) {
		case 'a':
			appnd++;
			break;
		case 'f':
			flush++;
			break;
		case 'y':
			syncf++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		usage();

	if (strcmp(argv[0], "flush") == 0) {
		if (argc != 1 || flush || appnd || syncf)
			usage();
		cmd = CMD_FLUSH;
	} else if (strcmp(argv[0], "flows") == 0) {
		const char *errstr;

		if (argc > 2 || flush || appnd || syncf)
			usage();
		if (argc == 2) {
			nflows = strtonum(argv[1], IPFLOW_MIN_FLOWS,
			    IPFLOW_MAX_FLOWS, &errstr);
			if (errstr != NULL)
				errx(EX_CONFIG, "%s: %s", argv[1], errstr);
		}
		cmd = CMD_FLOWS;
	} else if (strcmp(argv[0], "show") == 0) {
		if (argc != 1 || appnd || syncf)
			usage();
		cmd = CMD_SHOW;
	} else if (strcmp(argv[0], "load") == 0) {
		if (argc != 2 || appnd || syncf)
			usage();
		file = argv[1];
		cmd = CMD_LOAD;
	} else if (strcmp(argv[0], "save") == 0) {
		if (argc != 2)
			usage();
		file = argv[1];
		cmd = CMD_SAVE;
	} else if (strcmp(argv[0], "add") == 0) {
		if (argc < 2 || flush || appnd || syncf)
			usage();
		if (argc > 2 && strcmp(argv[2], "-F") == 0) {
			if (argc != 4)
				usage();
			file = argv[3];
		}
		cmd = CMD_ADD;
	} else if (strcmp(argv[0], "set") == 0) {
		if (argc < 2 || flush || appnd || syncf)
			usage();
		if (argc > 2 && strcmp(argv[2], "-F") == 0) {
			if (argc != 4)
				usage();
			file = argv[3];
		}
		cmd = CMD_SET;
	} else if (strcmp(argv[0], "del") == 0) {
		if (argc < 2 || flush || appnd || syncf)
			usage();
		cmd = CMD_DEL;
	} else if (strcmp(argv[0], "info") == 0) {
		if (flush || appnd || syncf)
			usage();
		cmd = CMD_INFO;
	} else if (strcmp(argv[0], "reset") == 0) {
		if (argc < 2 || flush || appnd || syncf)
			usage();
		cmd = CMD_RESET;
	} else if (strcmp(argv[0], "version") == 0) {
		if (argc != 1 || flush || appnd || syncf)
			usage();
		cmd = CMD_VERSION;
	} else
		usage();

	bzero(&iir, sizeof(iir));
	if ((dd = open(_PATH_DEV_IPFLOW, O_RDWR)) < 0 && errno == EACCES &&
	    (dd = open(_PATH_DEV_IPFLOW, O_RDONLY)) < 0)
		err(EX_UNAVAILABLE, "open: %s", _PATH_DEV_IPFLOW);

	if (ioctl(dd, IIOCVERSION, &iv) < 0)
		err(EX_IOERR, "IIOCVERSION");

	if (iv.iv_major != IPFLOW_VERSION_MAJOR && cmd != CMD_VERSION)
		errx(EX_CONFIG, "Unsupported ipflow version");

	switch (cmd) {
	case CMD_FLUSH:
		if (ioctl(dd, IIOCFFLOWS) < 0)
			err(EX_IOERR, "IIOCFFLOWS");
		break;
	case CMD_FLOWS:
		if (nflows == 0) {
			if (ioctl(dd, IIOCGINFO, &ifi) < 0)
				err(EX_IOERR, "IIOCGINFO");
			printf("recv %u, drop %u, max %u\n",
			    ifi.ifi_recv, ifi.ifi_drop, ifi.ifi_max);
		} else if (ioctl(dd, IIOCSNFLOWS, &nflows) < 0)
			err(EX_IOERR, "IIOCSNFLOWS");
		break;
	case CMD_SHOW:
	case CMD_LOAD:
	case CMD_SAVE:
		if (ioctl(dd, IIOCGNFLOWS, &irq.irq_nflows) < 0)
			err(EX_IOERR, "IIOCGNFLOWS");
		if ((irq.irq_flows = malloc(irq.irq_nflows *
		    sizeof(struct ipflow))) == NULL)
			err(EX_UNAVAILABLE, "malloc");
		switch (cmd) {
		case CMD_LOAD:
			if ((fd = open(file, O_RDONLY)) < 0)
				err(EX_UNAVAILABLE, "open: %s", file);
			while ((n = read(fd, irq.irq_flows,
			    irq.irq_nflows *  sizeof(struct ipflow))) > 0) {
				if (flush) {
					irq.irq_nflows =
					    n / sizeof(struct ipflow);
					if (ioctl(dd, IIOCSFLOWS, &irq) < 0)
						err(EX_IOERR, "IIOCSFLOWS");
				} else {
					if (write(dd, irq.irq_flows, n) < 0) {
						if (errno == EBADF)
							errno = EPERM;
						err(EX_IOERR, "write: %s",
						    _PATH_DEV_IPFLOW);
					}
				}
			}
			if (n < 0)
				err(EX_IOERR, "read: %s", file);
			break;
		case CMD_SAVE:
			ch = appnd ? O_WRONLY | O_APPEND | O_CREAT :
			    O_WRONLY | O_CREAT | O_TRUNC;
			if ((fd = open(file, ch, SAVE_FILE_MODE)) < 0)
				err(EX_UNAVAILABLE, "open: %s", file);
			/* FALLTHROUGH */
		case CMD_SHOW:
			if (flush) {
				if (ioctl(dd, IIOCGFLOWS, &irq) < 0)
					err(EX_IOERR, "IIOCGFLOWS");
				n = irq.irq_nflows * sizeof(struct ipflow);
			} else {
				if ((n = read(dd, irq.irq_flows,
				    irq.irq_nflows *
				    sizeof(struct ipflow))) < 0)
					err(EX_IOERR, "read: %s",
					    _PATH_DEV_IPFLOW);
				irq.irq_nflows = n / sizeof(struct ipflow);
			}
			if (cmd == CMD_SAVE) {
				if (write(fd, irq.irq_flows, n) < 0)
					err(EX_IOERR, "write: %s", file);
				if (syncf && fsync(fd) < 0)
					err(EX_IOERR, "fsync: %s", file);
				break;
			}
			for (n = 0; n < irq.irq_nflows; n++) {
				char first[20], last[20];

				(void)strftime(first, sizeof(first),
				    "%Y-%m-%d %H:%M:%S",
				    localtime(&irq.irq_flows[n].if_first));
				(void)strftime(last, sizeof(last),
				    "%Y-%m-%d %H:%M:%S",
				    localtime(&irq.irq_flows[n].if_last));
				(void)printf("%s|%s|%s|", first, last,
				    inet_ntoa(*(struct in_addr *)
				        &irq.irq_flows[n].if_src));
				(void)printf("%s|%llu|%llu\n",
				    inet_ntoa(*(struct in_addr *)
					&irq.irq_flows[n].if_dst),
				    irq.irq_flows[n].if_pkts,
				    irq.irq_flows[n].if_octets);
			}
			break;
		}
		break;
	case CMD_ADD:
	case CMD_SET:
		(void)strlcpy(iir.iir_name, argv[1], sizeof(iir.iir_name));
		if (cmd == CMD_ADD && ioctl(dd, IIOCADDIF, &iir) < 0)
			err(EX_IOERR, "IIOCADDIF: %s", iir.iir_name);
		if (ioctl(dd, IIOCGIFCONF, &iir) < 0)
			err(EX_IOERR, "IIOCGIFCONF: %s", iir.iir_name);
		filter = file == NULL ? copy_argv(argv + 2) : copy_file(file);
		if (pcap_compile_nopcap(96, iir.iir_dlt, &bprog,
		   filter, 1, 0) < 0)
			errx(EX_CONFIG, "Couldn't compile filter expression");
		iir.iir_bprog = (caddr_t)&bprog;
		if (ioctl(dd, IIOCSETF, &iir) < 0)
			err(EX_IOERR, "IIOCSETF: %s", iir.iir_name);
		break;
	case CMD_DEL:
		for (ch = 1; ch < argc; ch++) {
			(void)strlcpy(iir.iir_name, argv[ch],
			    sizeof(iir.iir_name));
			if (ioctl(dd, IIOCDELIF, &iir) < 0)
				warn("IIOCDELIF: %s", iir.iir_name);
		}
		break;
	case CMD_RESET:
		for (ch = 1; ch < argc; ch++) {
			(void)strlcpy(iir.iir_name, argv[ch],
			    sizeof(iir.iir_name));
			if (ioctl(dd, IIOCFLUSHIF, &iir) < 0)
				warn("IIOCFLUSHIF: %s", iir.iir_name);
		}
		break;
	case CMD_INFO:
		if (argc == 1) {
			if (ioctl(dd, IIOCGIFLIST, &iir) < 0)
				err(EX_IOERR, "IIOCGIFLIST");
			if (iir.iir_count == 0)
				break;
			if ((iir.iir_data = malloc(iir.iir_count *
			    sizeof(iir))) == NULL)
				err(EX_UNAVAILABLE, "malloc");
			if (ioctl(dd, IIOCGIFLIST, &iir) < 0 &&
			    errno != ENOMEM)
				err(EX_IOERR, "IIOCGIFLIST");
			for (n = 0; n < iir.iir_count; n++) {
				print_if((struct ipflow_ifreq *)iir.iir_data);
				iir.iir_data += sizeof(iir);
			}
			break;
		}
		for (ch = 1; ch < argc; ch++) {
			(void)strlcpy(iir.iir_name, argv[ch],
			    sizeof(iir.iir_name));
			if (ioctl(dd, IIOCGIFCONF, &iir) < 0) {
				warn("IIOCGIFCONF: %s", iir.iir_name);
				continue;
			}
			print_if(&iir);
		}
		break;
	case CMD_VERSION:
		printf("ipflow version %u.%u\n", iv.iv_major, iv.iv_minor);
		break;
	}

	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s flush\n"
	    "       %s flows [nflows]\n"
	    "       %s [-f] show\n"
	    "       %s [-f] load file\n"
	    "       %s [-afy] save file\n"
	    "       %s add interface [-F file | expression]\n"
	    "       %s set interface [-F file | expression]\n"
	    "       %s del interface [...]\n"
	    "       %s reset interface [...]\n"
	    "       %s info [interface [...]]\n"
	    "       %s version\n",
	    __progname, __progname, __progname, __progname, __progname,
	    __progname, __progname, __progname, __progname, __progname,
	    __progname);
	exit(EX_USAGE);
}

static char *
copy_argv(char * const argv[])
{
	int i, len = 0;
	char *buf;

	if (argv == NULL)
		return (NULL);

	for (i = 0; argv[i] != NULL; i++)
		len += strlen(argv[i]) + 1;
	if (len == 0)
		return (NULL);

	if ((buf = malloc(len)) == NULL)
		err(EX_UNAVAILABLE, "malloc");

	(void)strlcpy(buf, argv[0], len);
	for (i = 1; argv[i] != NULL; i++) {
		(void)strlcat(buf, " ", len);
		(void)strlcat(buf, argv[i], len);
	}

	return (buf);
}

static char *
copy_file(const char *path)
{
	struct stat st;
	ssize_t nbytes;
	char *cp;
	int fd;

	if ((fd = open(path, O_RDONLY)) < 0)
		err(EX_UNAVAILABLE, "open: %s", path);
	if (fstat(fd, &st) < 0)
		err(EX_UNAVAILABLE, "stat: %s", path);
	if ((cp = malloc((size_t)st.st_size + 1)) == NULL)
		err(EX_UNAVAILABLE, "malloc");
	if ((nbytes = read(fd, cp, (size_t)st.st_size)) < 0)
		err(EX_IOERR, "read: %s", path);
	if ((size_t)nbytes != st.st_size)
		errx(EX_IOERR, "%s: Short read", path);
	cp[(int)st.st_size] = '\0';

	return (cp);
}

static char *
link_type(u_int dlt)
{
	static struct link_type link_types[] = {
		{ DLT_NULL,		"NULL" },
		{ DLT_EN10MB,		"EN10MB" },
		{ DLT_EN3MB,		"EN3MB" },
		{ DLT_AX25,		"AX25" },
		{ DLT_PRONET,		"PRONET" },
		{ DLT_CHAOS,		"CHAOS" },
		{ DLT_IEEE802,		"IEEE802" },
		{ DLT_ARCNET,		"ARCNET" },
		{ DLT_SLIP,		"SLIP" },
		{ DLT_PPP,		"PPP" },
		{ DLT_FDDI,		"FDDI" },
		{ DLT_ATM_RFC1483,	"ATM_RFC1483" },
		{ DLT_LOOP,		"LOOP" },
		{ DLT_ENC,		"ENC" },
		{ DLT_RAW,		"RAW" },
		{ DLT_SLIP_BSDOS,	"SLIP_BSDOS" },
		{ DLT_PPP_BSDOS,	"PPP_BSDOS" },
		{ DLT_OLD_PFLOG,	"OLD_PFLOG" },
		{ DLT_PFSYNC,		"PFSYNC" },
		{ DLT_PPP_ETHER,	"PPP_ETHER" },
		{ DLT_IEEE802_11,	"IEEE802_11" },
		{ DLT_PFLOG,		"PFLOG" },
#ifdef DLT_IEEE802_11_RADIO
		{ DLT_IEEE802_11_RADIO,	"IEEE802_11_RADIO" },
#endif
		{ 0,			NULL }
	};
	static char buf[6];
	int i;

	for (i = 0; link_types[i].type != NULL; i++)
		if (link_types[i].dlt == dlt)
			return (link_types[i].type);
	(void)snprintf(buf, sizeof(buf), "%u", dlt);

	return (buf);
}

static void
print_if(struct ipflow_ifreq *iir)
{
	(void)printf("%s:\t%s, bpf %u, pid %d, recv %u, drop %u\n",
	    iir->iir_name, link_type(iir->iir_dlt), minor(iir->iir_dev),
	    iir->iir_pid, iir->iir_recv, iir->iir_drop);
}
