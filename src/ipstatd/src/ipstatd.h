/*
 *              $Id$
 */

#define MAXENTRY        8192
#define IPLEN           16
#define BACKETLEN       64
#define MAXPORT         1024
#define LOADSTATENTRY   1024
#define KEEPLOAD_PERIOD 5

typedef struct {
                u_int   packets;
                u_int   bytes;
        } counters;

typedef struct {
        u_int   in_packets;
        u_int   in_bytes;
        u_int   out_packets;
        u_int   out_bytes;
        } miscstat_t;

typedef struct trafstat{
                u_int   from;
                u_int   to;
                u_int   packets;
                u_int   bytes;
        } trafstat_t;

typedef counters protostat_t;

typedef struct {
                u_int   in_from_packets;
                u_int   in_from_bytes;
                u_int   out_from_packets;
                u_int   out_from_bytes;
                u_int   in_to_packets;
                u_int   in_to_bytes;
                u_int   out_to_packets;
                u_int   out_to_bytes;
	} portstat_t;


