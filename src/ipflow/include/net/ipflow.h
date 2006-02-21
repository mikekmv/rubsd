/*	$RuOBSD: ipflow.h,v 1.2 2005/12/11 05:12:22 form Exp $	*/

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

#ifndef __NET_IPFLOW_H__
#define __NET_IPFLOW_H__

#define IPFLOW_VERSION_MAJOR	1
#define IPFLOW_VERSION_MINOR	4

#define _PATH_DEV_IPFLOW	"/dev/ipflow"
#define _MODE_DEV_IPFLOW	0600

#define IPFLOW_MIN_FLOWS	512
#define IPFLOW_DEF_FLOWS	10240
#define IPFLOW_MAX_FLOWS	102400


struct ipflow {
	time_t			if_first;
	time_t			if_last;
	in_addr_t		if_src;
	in_addr_t		if_dst;
	u_int64_t		if_pkts;
	u_int64_t		if_octets;
};

struct ipflow_info {
	u_int			ifi_recv;
	u_int			ifi_drop;
	u_int			ifi_max;
	pid_t			ifi_pid;
};

struct ipflow_req {
	struct ipflow		*irq_flows;
	u_int			irq_nflows;
};

struct ipflow_ifreq {
	char			iir_name[IFNAMSIZ];
	caddr_t			iir_data;
	u_int			iir_count;
	dev_t			iir_dev;
	u_int			iir_dlt;
	u_int			iir_recv;
	u_int			iir_drop;
	pid_t			iir_pid;
#define iir_bprog		iir_data
};

struct ipflow_version {
	u_int			iv_major;
	u_int			iv_minor;
};


#define IIOCFFLOWS		_IO('I', 242)
#define IIOCGFLOWS		_IOWR('I', 243, struct ipflow_req)
#define IIOCSFLOWS		_IOW('I', 244, struct ipflow_req)
#define IIOCGNFLOWS		_IOR('I', 245, u_int)
#define IIOCSNFLOWS		_IOW('I', 246, u_int)
#define IIOCADDIF		_IOW('I', 247, struct ipflow_ifreq)
#define IIOCDELIF		_IOW('I', 248, struct ipflow_ifreq)
#define IIOCSETF		_IOW('I', 249, struct ipflow_ifreq)
#define IIOCGIFCONF		_IOWR('I', 250, struct ipflow_ifreq)
#define IIOCGIFLIST		_IOWR('I', 251, struct ipflow_ifreq)
#define IIOCFLUSHIF		_IOW('I', 252, struct ipflow_ifreq)
#define IIOCVERSION		_IOR('I', 253, struct ipflow_version)
#define IIOCGINFO		_IOR('I', 254, struct ipflow_info)


#ifdef _KERNEL
struct ipflow_entry {
	struct ipflow		ife_flow;
	RB_ENTRY(ipflow_entry)	ife_entry;
#define ife_first		ife_flow.if_first
#define ife_last		ife_flow.if_last
#define ife_src			ife_flow.if_src
#define ife_dst			ife_flow.if_dst
#define ife_pkts		ife_flow.if_pkts
#define ife_octets		ife_flow.if_octets
};

RB_HEAD(ipflow_tree, ipflow_entry);

typedef void			(*ipflow_collector)(void *, int);

struct ipflow_if {
	char			ii_name[IFNAMSIZ];
	struct proc		*ii_proc;
	void			*ii_buf;
	int			ii_len;
	dev_t			ii_dev;
	int			ii_dlt;
	ipflow_collector	ii_collect;
	int			ii_error;
	LIST_ENTRY(ipflow_if)	ii_list;
};

LIST_HEAD(ipflow_if_list, ipflow_if);


extern struct cdevsw ipflow_cdevsw;
extern struct proc *ipflow_proc;
extern struct selinfo ipflow_rsel;
extern struct selinfo ipflow_wsel;
extern struct ipflow_if_list ipflow_if_list;
extern struct ipflow_info ipflow_info;
extern struct ipflow_tree ipflow_tree;
extern struct ipflow_entry *ipflow_entries;


#define ipflow_nflows		ipflow_info.ifi_recv
#define ipflow_dropped		ipflow_info.ifi_drop
#define ipflow_maxflows		ipflow_info.ifi_max
#define ipflow_pid		ipflow_info.ifi_pid
#define ipflow_apid		ipflow_info.ifi_apid


static __inline int
ipflow_compare(struct ipflow_entry *ife1, struct ipflow_entry *ife2)
{
	if (ife1->ife_src < ife2->ife_src)
		return (-1);
	if (ife1->ife_src > ife2->ife_src)
		return (1);
	if (ife1->ife_dst < ife2->ife_dst)
		return (-1);
	if (ife1->ife_dst > ife2->ife_dst)
		return (1);
	return (0);
}


RB_PROTOTYPE(ipflow_tree, ipflow_entry, ife_entry, ipflow_compare)

cdev_decl(ipflow);

int ipflowattach(void);
void ipflowdetach(void);

void ipflowd_master(void *);
void ipflowd_iface(void *);

int ipflow_init(void);
int ipflow_realloc(u_int);
void ipflow_free(void);
void ipflow_flush(void);
void ipflow_collect(void *, int);
void ipflow_insert(struct ipflow *);
ipflow_collector ipflow_lookup_collector(int);

struct ipflow_if *ipflow_if_lookup(const char *);
int ipflow_if_create(const char *);
void ipflow_if_destroy(struct ipflow_if *);
#endif	/* _KERNEL */

#endif	/* __NET_IPFLOW_H__ */
