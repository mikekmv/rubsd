/*	$Id$	*/

#define MAXENTRY        8192
#define IPLEN           16
#define BACKETLEN       64
#define MAXPORT         1024
#define LOADSTATENTRY   1024
#define KEEPLOAD_PERIOD 5

struct counters {
	u_int   packets;
	u_int   bytes;
} counters;

struct miscstat {
        u_int   in_packets;
        u_int   in_bytes;
        u_int   out_packets;
        u_int   out_bytes;
} miscstat;

struct trafstat {
	u_int   from;
	u_int   to;
	u_int   packets;
	u_int   bytes;
} trafstat;

struct portstat {
	u_int   in_from_packets;
	u_int   in_from_bytes;
	u_int   out_from_packets;
	u_int   out_from_bytes;
	u_int   in_to_packets;
	u_int   in_to_bytes;
	u_int   out_to_packets;
	u_int   out_to_bytes;
} portstat;

struct packdesc {
	char	ifname[IFNAMSIZ];
	u_int	count;
	u_int	flags;
	u_int	plen;
	struct ip	*ip;
} packdesc;

/* Packet flags */
#define	P_OUTPUT	0x00000001	/* Packet outgoing from interface */
#define	P_PASS		0x00000002	/* Packet passed through firewall */
#define	P_BLOCK		0x00000004	/* Packet blocked by firewall */
#define	P_SHORT		0x00000008	/* Short packet */
