/*	$RuOBSD: collect.h,v 1.6 2004/11/13 17:57:53 form Exp $	*/

/*
 * Copyright (c) 2003-2004 Oleg Safiullin <form@pdp-11.org.ru>
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

#ifndef __COLLECT_H__
#define __COLLECT_H__

#ifdef NEED_SYS_ENDIAN
#include <sys/endian.h>
#endif

#define MIN_CT_ENTRIES	128
#define DEF_CT_ENTRIES	5000
#define MAX_CT_ENTRIES	131072

#ifndef htobe32
#define htobe32(x)	htonl(x)
#endif

#ifndef betoh32
#define betoh32(x)	ntohl(x)
#endif

#ifndef htobe64
#if BYTE_ORDER == LITTLE_ENDIAN
#define htobe64(x) __extension__({			\
	u_int64_t _x = (x);				\
							\
	(u_int64_t)((_x & 0xffULL) << 56 |			\
	    (_x & 0xff00ULL) << 40 |			\
	    (_x & 0xff0000ULL) << 24 |			\
	    (_x & 0xff000000ULL) << 8 |			\
	    (_x & 0xff00000000ULL) >> 8 |		\
	    (_x & 0xff0000000000ULL) >> 24 |		\
	    (_x & 0xff000000000000ULL) >> 40 |		\
	    (_x & 0xff00000000000000ULL) >> 56);	\
})
#endif	/* BYTE_ORDER == LITTLE_ENDIAN */

#if BYTE_ORDER == BIG_ENDIAN
#define htobe64(x) (x)
#endif	/* BYTE_ORDER == BIG_ENDIAN */
#endif	/* htobe64 */

#ifndef betoh64
#define betoh64(x)	htobe64(x)
#endif

union uniaddr {
	struct in_addr	ua_in;
	struct in6_addr	ua_in6;
#ifndef s6_addr32
#define s6_addr32	__u6_addr.__u6_addr32
#endif
};

struct coll_header {
	u_int32_t	ch_flags;
#define CNUPM_MAJOR(x)	((x) & 0xFF)
#define CNUPM_MINOR(x)	(((x) >> 8) & 0xFF)
	time_t		ch_start;
	time_t		ch_stop;
	u_int32_t	ch_count;
};

struct coll_traffic {
	sa_family_t	ct_family;
	u_int8_t	ct_proto;
	in_port_t	ct_sport;
	in_port_t	ct_dport;
	union uniaddr	ct_src;
	union uniaddr	ct_dst;
	u_int64_t	ct_bytes;
};

extern int		ct_entries_max;
extern u_int32_t	collect_lost_packets;
extern int		collect_need_dump;
extern sa_family_t	collect_family;
extern int		collect_proto;
extern int		collect_ports;

__BEGIN_DECLS
int	collect_init(int);
void	collect(sa_family_t, const void *);
int	collect_dump(const char *, int, int, int);
__END_DECLS

#endif	/* __COLLECT_H__ */
