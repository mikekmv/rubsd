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

#ifndef __COLLECT_H__
#define __COLLECT_H__

#ifdef INET6
#define s6_addr32 __u6_addr.__u6_addr32
#endif

struct ic_addr {
	union {
		struct in_addr	ica_in4;
#ifdef INET6
		struct in6_addr	ica_in6;
#endif
	} ica_addr;
#define ica_in4	ica_addr.ica_in4
#ifdef INET6
#define ica_in6	ica_addr.ica_in6
#endif
};

struct ic_traffic {
	sa_family_t		ict_family;
	u_int8_t		ict_proto;
	struct ic_addr		ict_src;
	struct ic_addr		ict_dst;
#ifndef NOPORTS
	in_port_t		ict_sport;
	in_port_t		ict_dport;
#endif
	u_int64_t		ict_bytes;
};

struct ic_header {
	time_t			ich_time;
	u_int32_t		ich_length;
};

__BEGIN_DECLS
int	ic_init(void);
int	ic_collect(sa_family_t, const void *);
int	ic_dump(const char *);
__END_DECLS

#endif	/* __COLLECT_H__ */
