/* $RuOBSD: ipstatd.h,v 1.10 2002/03/15 11:40:20 tm Exp $	 */

#include <sys/types.h>

#define MAXENTRY        8192
#define IPLEN           16
#define BACKETLEN       64
#define MAXPORT         1024
#define LOADSTATENTRY   1024
#define KEEPLOAD_PERIOD 5


struct counters {
	u_int64_t       packets;
	u_int64_t       bytes;
};

struct miscstat {
	u_int64_t       in_packets;
	u_int64_t       in_bytes;
	u_int64_t       out_packets;
	u_int64_t       out_bytes;
};

struct trafstat {
	u_int64_t       from;
	u_int64_t       to;
	u_int64_t       packets;
	u_int64_t       bytes;
};

struct portstat {
	u_int64_t       in_from_packets;
	u_int64_t       in_from_bytes;
	u_int64_t       out_from_packets;
	u_int64_t       out_from_bytes;
	u_int64_t       in_to_packets;
	u_int64_t       in_to_bytes;
	u_int64_t       out_to_packets;
	u_int64_t       out_to_bytes;
};

struct capture {
	int             (*open) (void);
	void            (*read) (void);
	void            (*close) (void);
};

struct packdesc {
	char            ifname[IFNAMSIZ];
	u_int64_t       count;
	u_int           flags;
	u_int           plen;
	struct ip      *ip;
};

/* Packet flags */
#define	P_OUTPUT	0x00000001	/* Packet outgoing from interface */
#define	P_PASS		0x00000002	/* Packet passed through firewall */
#define	P_BLOCK		0x00000004	/* Packet blocked by firewall */
#define	P_SHORT		0x00000008	/* Short packet */

int             parse_ip(struct packdesc *);
