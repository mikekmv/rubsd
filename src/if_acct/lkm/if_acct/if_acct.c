/*	$RuOBSD: if_acct.c,v 1.5 2004/10/28 05:49:46 form Exp $	*/

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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/tree.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_acct.h>
#if NBPFILTER > 0
#include <net/bpf.h>
#endif

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

struct acct_entry {
	RB_ENTRY(acct_entry)	ae_entry;
	struct acct_flow	ae_flow;
#define ae_first		ae_flow.af_first
#define ae_last			ae_flow.af_last
#define ae_pkts			ae_flow.af_pkts
#define ae_octets		ae_flow.af_octets
#define ae_src			ae_flow.af_src
#define ae_dst			ae_flow.af_dst
#define ae_sport		ae_flow.af_sport
#define ae_dport		ae_flow.af_dport
#define ae_proto		ae_flow.af_proto
};

RB_HEAD(acct_tree, acct_entry);

struct acct_softc {
	struct ifnet		as_if;
	u_int32_t		as_flags;
	int			as_nflows;
	struct acct_entry	*as_entries;
	struct acct_tree	as_tree;
	int			as_unit;
	LIST_ENTRY(acct_softc)	as_list;
};


static __inline int
acct_compare(struct acct_entry *a, struct acct_entry *b)
{
	if (a->ae_src < b->ae_src)
		return (-1);
	if (a->ae_src > b->ae_src)
		return (1);
	if (a->ae_dst < b->ae_dst)
		return (-1);
	if (a->ae_dst > b->ae_dst)
		return (1);
	return (0);
}


RB_PROTOTYPE(acct_tree, acct_entry, ae_entry, acct_compare)

static int acct_clone_create(struct if_clone *, int);
static int acct_clone_destroy(struct ifnet *);

static int acct_ioctl(struct ifnet *, u_long, caddr_t);
static int acct_output(struct ifnet *, struct mbuf *, struct sockaddr *,
    struct rtentry *);
static void acct_start(struct ifnet *);
static void acct_init(struct ifnet *);
static void acct_stop(struct ifnet *);


extern int ifqmaxlen;

LIST_HEAD(, acct_softc) acct_softc_list;

struct if_clone acct_cloner =
    IF_CLONE_INITIALIZER("acct", acct_clone_create, acct_clone_destroy);


RB_GENERATE(acct_tree, acct_entry, ae_entry, acct_compare)

static int
acct_clone_create(struct if_clone *ifc, int unit)
{
	struct acct_softc *as;
	struct ifnet *ifp;
	int s;

	if ((as = malloc(sizeof(*as), M_DEVBUF, M_NOWAIT)) == NULL)
		return (ENOMEM);
	bzero(as, sizeof(*as));

	as->as_unit = unit;

	ifp = &as->as_if;
	(void)snprintf(ifp->if_xname, sizeof(ifp->if_xname), "%s%d",
	    ifc->ifc_name, unit);
	ifp->if_softc = as;
	ifp->if_ioctl = acct_ioctl;
	ifp->if_output = acct_output;
	ifp->if_start = acct_start;
	IFQ_SET_MAXLEN(&ifp->if_snd, ifqmaxlen);
	IFQ_SET_READY(&ifp->if_snd);
	ifp->if_mtu = ACCTMTU;
	ifp->if_type  = IFT_PROPVIRTUAL;
	if_attach(ifp);
	if_alloc_sadl(ifp);
#if NBPFILTER > 0
	bpfattach(&ifp->if_bpf, ifp, DLT_RAW, 0);
#endif

	s = splnet();
	LIST_INSERT_HEAD(&acct_softc_list, as, as_list);
	splx(s);

	return (0);
}

static int
acct_clone_destroy(struct ifnet *ifp)
{
	struct acct_softc *as = ifp->if_softc;
	int s;

	acct_stop(ifp);

	s = splimp();
	LIST_REMOVE(as, as_list);
	splx(s);

#if NBPFILTER > 0
	bpfdetach(ifp);
#endif
	if_detach(ifp);
	free(as, M_DEVBUF);

	return (0);
}

int
acct_attach(void)
{
	LIST_INIT(&acct_softc_list);
	if_clone_attach(&acct_cloner);

	return (0);
}

int
acct_detach(void)
{
	struct acct_softc *as;
	int s;


	s = splimp();
	LIST_FOREACH(as, &acct_softc_list, as_list) {
		if (as->as_if.if_flags & IFF_RUNNING) {
			splx(s);
			return (EBUSY);
		}
	}
	splx(s);

	if_clone_detach(&acct_cloner);

	s = splimp();
	while ((as = LIST_FIRST(&acct_softc_list)) != NULL)
		acct_clone_destroy(&as->as_if);
	splx(s);

	return (0);
}

static int
acct_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	struct acct_softc *as = ifp->if_softc;
	struct ifreq *ifr = (struct ifreq *)data;
	struct acctio_flows aif;
	struct acct_flow *af;
	struct acct_entry *ae;
	int s, error = 0;

	s = splimp();

	switch (cmd) {
	case SIOCSIFFLAGS:
		if (ifp->if_flags & IFF_UP)
			acct_init(ifp);
		else
			acct_stop(ifp);
		break;
	case SIOCGFLOWS:
	case SIOCGRFLOWS:
		if ((error = suser(curproc, 0)) != 0)
			break;

		if ((error = copyin(ifr->ifr_data, &aif, sizeof(aif))) != 0)
			break;

		if (aif.aif_nflows < as->as_nflows) {
			error = aif.aif_nflows <= 0 ? ENOMEM : EINVAL;
			break;
		}

		if ((aif.aif_nflows = as->as_nflows) != 0) {
			af = aif.aif_flows;
			RB_FOREACH(ae, acct_tree, &as->as_tree) {
				error = copyout(&ae->ae_flow, af, sizeof(*af));
				if (error != 0)
					break;
				af++;
			}
		}

		if (error == 0)
			error = copyout(&aif, ifr->ifr_data, sizeof(aif));

		if (error != 0 || cmd != SIOCGRFLOWS)
			break;

		/* FALLTHROUGH */
	case SIOCRFLOWS:
		if ((error = suser(curproc, 0)) != 0)
			break;

		if (as->as_nflows != 0) {
			RB_INIT(&as->as_tree);
			as->as_nflows = 0;
		}
		ifp->if_ipackets = ifp->if_ierrors = ifp->if_ibytes = 0;
		break;
	default:
		error = EINVAL;
		break;
	}

	splx(s);

	return (error);
}

static int
acct_output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *dst,
    struct rtentry *rt)
{
	struct acct_softc *as = ifp->if_softc;
	struct ip *ip = mtod(m, struct ip *);
	struct acct_entry *ae, aef;
	int s;

	if ((ifp->if_flags & IFF_RUNNING) && dst->sa_family == AF_INET &&
	    m->m_len >= sizeof(struct ip)) {
		bzero(&aef, sizeof(aef));
		aef.ae_src = ip->ip_src.s_addr;
		aef.ae_dst = ip->ip_dst.s_addr;
		aef.ae_pkts++;
		aef.ae_octets = m->m_pkthdr.len;
		aef.ae_first = aef.ae_last = time.tv_sec;
		s = splnet();
		if ((ae = RB_FIND(acct_tree, &as->as_tree, &aef)) == NULL) {
			if (as->as_nflows < ACCTFLOWS) {
				as->as_entries[as->as_nflows] = aef;
				RB_INSERT(acct_tree, &as->as_tree,
				    &as->as_entries[as->as_nflows]);
				as->as_nflows++;
			} else {
				printf("%s: IP flows tree full\n",
				    ifp->if_xname);
				ifp->if_ierrors++;
				ifp->if_oerrors++;
			}
		} else {
			ae->ae_pkts++;
			ae->ae_octets += aef.ae_octets;
			ae->ae_last = aef.ae_last;
		}
		splx(s);
		ifp->if_ibytes += m->m_pkthdr.len;
		ifp->if_obytes += m->m_pkthdr.len;
		ifp->if_ipackets++;
		ifp->if_opackets++;
#if NBPFILTER > 0
		if (ifp->if_bpf != NULL)
			bpf_mtap(ifp->if_bpf, m);
#endif
	}

	m_freem(m);

	return (0);
}

static void
acct_start(struct ifnet *ifp)
{
	struct mbuf *m;
	int s;

	for (;;) {
		s = splimp();
		IFQ_DEQUEUE(&ifp->if_snd, m);
		splx(s);

		if (m == NULL)
			break;

		m_freem(m);
	}
}

static void
acct_init(struct ifnet *ifp)
{
	struct acct_softc *as = ifp->if_softc;

	if (!(ifp->if_flags & IFF_RUNNING)) {
		RB_INIT(&as->as_tree);
		as->as_nflows = 0;
		as->as_entries = malloc(sizeof(struct acct_entry) * ACCTFLOWS,
		    M_DEVBUF, M_NOWAIT);
		if (as->as_entries == NULL) {
			printf("%s: couldn't initialize IP flows tree\n",
			    ifp->if_xname);
			return;
		}
		ifp->if_flags |= IFF_RUNNING;
	}
}

static void
acct_stop(struct ifnet *ifp)
{
	struct acct_softc *as = ifp->if_softc;

	if (ifp->if_flags & IFF_RUNNING) {
		free(as->as_entries, M_DEVBUF);
		as->as_nflows = 0;
		ifp->if_flags &= ~IFF_RUNNING;
	}
}
