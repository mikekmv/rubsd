/*	$RuOBSD$	*/

/*
 * Copyright (c) 2004 Oleg Safiullin <form@pdp-11.org.ru>
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

#ifndef __NET_IF_ACCT_H__
#define __NET_IF_ACCT_H__

#include <sys/ioccom.h>


#define ACCTFLOWS		5000

#define SIOCGFLOWS		_IOWR('i', 252, struct ifreq)
#define SIOCGRFLOWS		_IOWR('i', 253, struct ifreq)
#define SIOCRFLOWS		_IO('i', 254)


struct acct_flow {
	in_addr_t		af_src;
	in_addr_t		af_dst;
	time_t			af_first;
	time_t			af_last;
	u_int32_t		af_pkts;
	u_int32_t		af_octets;
};

struct acctio_flows {
	struct acct_flow	*aif_flows;
	int			aif_nflows;
};


#ifdef _KERNEL
#define ACCTMTU			33224


int	acct_attach(void);
int	acct_detach(void);
#endif	/* _KERNEL */

#endif	/* __NET_IF_ACCT_H__ */
