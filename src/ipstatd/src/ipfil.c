/*	$RuOBSD: ipfil.c,v 1.19 2002/03/22 17:44:10 grange Exp $	*/

const char ipfil_ver[] = "$RuOBSD: ipfil.c,v 1.19 2002/03/22 17:44:10 grange Exp $";

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_IPFILTER
#if HAVE_NETINET_IP_COMPAT_H
# include <netinet/ip_compat.h>
#else
# include <netinet/ip_fil_compat.h>
#endif
# include <netinet/ip_fil.h>
# include <netinet/ip_nat.h>
#endif

#include "ipstatd.h"

static void parse_ipl(char *, int);
static int  open_ipl(void);
static void read_ipl(void);
static void close_ipl(void);

struct capture ipl_cap = { open_ipl, read_ipl, close_ipl };

char *iplfile;
struct pollfd ipl_fds;

static int
open_ipl(void)
{
	iplfile = IPL_NAME;

	if ((ipl_fds.fd = open(iplfile, O_RDONLY)) == -1) {
		syslog(LOG_ERR, "%s: open: %m, exiting...", iplfile);
		exit(1);
	}

	return(0);
}

static void
read_ipl(void)
{
	int nr = 0;
	char buff[IPLLOGSIZE];
	char *bp = NULL, *bpo = NULL, *buf;
	iplog_t *ipl;
	int psize, blen;
	static time_t last_check = 0, cur_time;

	/*
	 * fucked ipfilter... If no packets in kernel log buffer
	 * then poll returns 1, but read blocks !
	 */
	ipl_fds.events = POLLIN;
	if (poll(&ipl_fds, 1, 100) <= 0)
		return;

	blen = read(ipl_fds.fd, buff, sizeof(buff));
	if (blen == -1) {
		syslog(LOG_ERR, "%s: read: %m\n", iplfile);
		stop();
	}
	if (blen) {
		buf=buff;
		while (blen > 0) {
			ipl = (iplog_t *)buf;
			if ((u_long)ipl & (sizeof(long) - 1)) {
				if (bp)
					bpo = bp;
				bp = (char *)malloc(blen);
				bcopy((char *)ipl, bp, blen);
				if (bpo) {
					free(bpo);
					bpo = NULL;
				}
				buf = bp;
				continue;
			}
			if (ipl->ipl_magic != IPL_MAGIC) {
				/* invalid data or out of sync */
				break;
			}
			psize = ipl->ipl_dsize;
			parse_ipl(buf, psize);
			blen -= psize;
			buf += psize;
			nr++;
		}
		if (bp) {
			free(bp);
			bp = NULL;
		}
	}

	cur_time = time(NULL);
	if (cur_time - last_check > 10) {
		if (chkiplovr())
			syslog(LOG_WARNING,
			   "Kernel ipl buffer overloaded, lost statistics");
		last_check = cur_time;
	}
}

static void
parse_ipl(char *buf, int blen)
{
	struct packdesc	pack;
	struct ip *ip;
	iplog_t *ipl;
	ipflog_t *ipf;

	ipl = (iplog_t *)buf;
	ipf = (ipflog_t *)((char *)buf + sizeof(*ipl));
	pack.plen = blen - sizeof(iplog_t) - sizeof(ipflog_t);

	ip = (struct ip *)((char *)ipf + sizeof(*ipf));
	pack.ip = ip;
	ip->ip_len = htons(ip->ip_len);
	ip->ip_off = htons(ip->ip_off);

	pack.flags = 0;
	if (ipf->fl_flags & FF_SHORT)
		pack.flags |= P_SHORT;
	if (ipf->fl_flags & FR_PASS)
		pack.flags |= P_PASS;
	if (ipf->fl_flags & FR_BLOCK)
		pack.flags |= P_BLOCK;
	if (ipf->fl_flags & FR_OUTQUE)
		pack.flags |= P_OUTPUT;

	pack.count = ipl->ipl_count;
#if DEBUG
	if(pack.count > 1)
		syslog(LOG_WARNING, "ipl_count = %d", pack.count);
#endif
	strncpy(pack.ifname, (const char*)ipf->fl_ifname, IFNAMSIZ);
	pack.ifname[IFNAMSIZ - 1] = '\0';

	parse_ip(&pack);
}

static int
chkiplovr(void)
{
	struct friostat frst;
	struct friostat *frstp = &frst;
	int count;
	static int ipl_skip = -1;

	if (ioctl(ipl_fds.fd, SIOCGETFS, &frstp) == -1) {
		syslog(LOG_ERR, "ioctl: %m");
		return (0);
	}
	count = frst.f_st[0].fr_skip + frst.f_st[1].fr_skip;
	if((ipl_skip < 0) || (ipl_skip > count)) {
		ipl_skip = count;
		return (0);
	}
	count -= ipl_skip;
	ipl_skip += count;
#if DEBUG
	syslog(LOG_DEBUG, "ipl_skip: %d, count: %d", ipl_skip, count);
#endif
	return (count);
}

static void
close_ipl(void)
{
	close(ipl_fds.fd);
}
