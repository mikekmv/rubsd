/*	$RuOBSD$	*/

/*
 * Copyright (c) 2003 Oleg Safiullin <form@pdp11.org.ru>
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

#ifndef htobe32
#define htobe32(x)	htonl(x)
#endif

#ifndef betoh32
#define betoh32(x)	ntohl(x)
#endif

#ifndef htobe64
#define htobe64(x) __extension__({		\
	u_int64_t _x = (x);			\
						\
	(u_int64_t)((_x & 0xff) << 56 |		\
	    (_x & 0xff00U) << 40 |		\
	    (_x & 0xff0000U) << 24 |		\
	    (_x & 0xff000000U) << 8 |		\
	    (_x & 0xff00000000U) >> 8 |		\
	    (_x & 0xff0000000000U) >> 24 |	\
	    (_x & 0xff000000000000U) >> 40 |	\
	    (_x & 0xff00000000000000U) >> 56);	\
})
#endif

#ifndef betoh64
#define betoh64(x)	htobe64(x)
#endif

union uniaddr {
	struct in_addr	ua_in;
#ifdef INET6
	struct in6_addr	ua_in6;
#define s6_addr32	__u6_addr.__u6_addr32
#endif
};

struct coll_header {
	u_int32_t	ch_flags;
	time_t		ch_start;
	time_t		ch_stop;
	u_int32_t	ch_count;
};

struct coll_traffic {
#ifdef INET6
	sa_family_t	ct_family;
#endif
#ifdef PROTO
	u_int8_t	ct_proto;
#endif
	union uniaddr	ct_src;
	union uniaddr	ct_dst;
#ifdef PORTS
	in_port_t	ct_sport;
	in_port_t	ct_dport;
#endif
	u_int64_t	ct_bytes;
};

extern u_int32_t	collect_lost_packets;
extern int		collect_need_dump;

__BEGIN_DECLS
int	collect_init(void);
void	collect(sa_family_t, const void *);
int	collect_dump(const char *);
__END_DECLS

#endif	/* __COLLECT_H__ */
