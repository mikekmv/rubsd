const char ipfil_ver[] = "$Id$";

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ipstatd.h"

void	parse_ipl(char*, int);

extern	char		*iplfile;
extern	int		iplfd;
extern	u_int		*backet_pass_len, *backet_block_len;
extern	struct trafstat	**backet_pass, **backet_block;
extern	struct miscstat	pass_stat, block_stat;
int	ipl_skip = -1;

void
read_ipl(int fd)
{
	int	nr = 0;
	char	buff[IPLLOGSIZE];
        char	*bp = NULL, *bpo = NULL, *buf;
        iplog_t *ipl;
        int	psize, blen;
	
	blen = read(fd, buff, sizeof(buff));	
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
}

void
parse_ipl(buf, blen)
	char	*buf;
	int	blen;
{
	struct packdesc	 pack;
        iplog_t 	*ipl;
        ipflog_t 	*ipf;

        ipl = (iplog_t *)buf;
        ipf = (ipflog_t *)((char *)buf + sizeof(*ipl));
        pack.ip = (ip_t *)((char *)ipf + sizeof(*ipf));
	pack.plen = blen - sizeof(iplog_t) - sizeof(ipflog_t);
	pack.flags = ipf->fl_flags;
	pack.count = ipl->ipl_count;
#if 0
	if(pack.count > 1)
		syslog(LOG_WARNING, "ipl_count = %d", pack.count);
#endif
	strncpy(pack.ifname, (const char*)ipf->fl_ifname, IFNAMSIZ);

	parse_ip(&pack);
}

int
chkiplovr(void)
{
	struct friostat	frst;
	int		count;

	if(ioctl(iplfd, SIOCGETFS, &frst) == -1)
		syslog(LOG_ERR, "ioctl: %m");
	count = frst.f_st[0].fr_skip + frst.f_st[1].fr_skip;
	if((ipl_skip < 0) || (ipl_skip > count)) {
		ipl_skip = count;
		return (0);
	}
	count -= ipl_skip;
	ipl_skip += count;
#if 0
	syslog(LOG_DEBUG, "ipl_skip: %d, count: %d", ipl_skip, count);
#endif
	return(count);
}
