/*	$OpenBSD: altq_red.c,v 1.16 2008/05/08 15:22:02 chl Exp $	*/
/*	$KAME: altq_red.c,v 1.10 2002/04/03 05:38:51 kjc Exp $	*/

/*
 * Copyright (C) 1997-2002
 *	Sony Computer Science Laboratories Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SONY CSL AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL SONY CSL OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/*
 * Copyright (c) 1990-1994 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/systm.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/if_types.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#ifdef INET6
#include <netinet/ip6.h>
#endif

#include <net/pfvar.h>
#include <altq/altq.h>
#include <altq/altq_red.h>

/*
 * ALTQ/RED (Random Early Detection) implementation using 32-bit
 * fixed-point calculation.
 *
 * written by kjc using the ns code as a reference.
 * you can learn more about red and ns from Sally's home page at
 * http://www-nrg.ee.lbl.gov/floyd/
 *
 * most of the red parameter values are fixed in this implementation
 * to prevent fixed-point overflow/underflow.
 * if you change the parameters, watch out for overflow/underflow!
 *
 * the parameters used are recommended values by Sally.
 * the corresponding ns config looks:
 *	q_weight=0.00195
 *	minthresh=5 maxthresh=15 queue-size=60
 *	linterm=30
 *	dropmech=drop-tail
 *	bytes=false (can't be handled by 32-bit fixed-point)
 *	doubleq=false dqthresh=false
 *	wait=true
 */
/*
 * alternative red parameters for a slow link.
 *
 * assume the queue length becomes from zero to L and keeps L, it takes
 * N packets for q_avg to reach 63% of L.
 * when q_weight is 0.002, N is about 500 packets.
 * for a slow link like dial-up, 500 packets takes more than 1 minute!
 * when q_weight is 0.008, N is about 127 packets.
 * when q_weight is 0.016, N is about 63 packets.
 * bursts of 50 packets are allowed for 0.002, bursts of 25 packets
 * are allowed for 0.016.
 * see Sally's paper for more details.
 */
/* normal red parameters */
#define	W_WEIGHT	512	/* inverse of weight of EWMA (511/512) */
				/* q_weight = 0.00195 */

/* red parameters for a slow link */
#define	W_WEIGHT_1	128	/* inverse of weight of EWMA (127/128) */
				/* q_weight = 0.0078125 */

/* red parameters for a very slow link (e.g., dialup) */
#define	W_WEIGHT_2	64	/* inverse of weight of EWMA (63/64) */
				/* q_weight = 0.015625 */

/* fixed-point uses 12-bit decimal places */
#define	FP_SHIFT	12	/* fixed-point shift */

/* red parameters for drop probability */
#define	INV_P_MAX	10	/* inverse of max drop probability */
#define	TH_MIN		5	/* min threshold */
#define	TH_MAX		15	/* max threshold */

#define	RED_LIMIT	60	/* default max queue length */
#define	RED_STATS		/* collect statistics */

/*
 * our default policy for forced-drop is drop-tail.
 * (in altq-1.1.2 or earlier, the default was random-drop.
 * but it makes more sense to punish the cause of the surge.)
 * to switch to the random-drop policy, define "RED_RANDOM_DROP".
 */

/* default red parameter values */
static int default_th_min = TH_MIN;
static int default_th_max = TH_MAX;
static int default_inv_pmax = INV_P_MAX;

/*
 * red support routines
 */
red_t *
red_alloc(int weight, int inv_pmax, int th_min, int th_max, int flags,
   int pkttime)
{
	red_t	*rp;
	int	 w, i;
	int	 npkts_per_sec;

	rp = malloc(sizeof(red_t), M_DEVBUF, M_WAITOK|M_ZERO);

	rp->red_avg = 0;
	rp->red_idle = 1;

	if (weight == 0)
		rp->red_weight = W_WEIGHT;
	else
		rp->red_weight = weight;
	if (inv_pmax == 0)
		rp->red_inv_pmax = default_inv_pmax;
	else
		rp->red_inv_pmax = inv_pmax;
	if (th_min == 0)
		rp->red_thmin = default_th_min;
	else
		rp->red_thmin = th_min;
	if (th_max == 0)
		rp->red_thmax = default_th_max;
	else
		rp->red_thmax = th_max;

	rp->red_flags = flags;

	if (pkttime == 0)
		/* default packet time: 1000 bytes / 10Mbps * 8 * 1000000 */
		rp->red_pkttime = 800;
	else
		rp->red_pkttime = pkttime;

	if (weight == 0) {
		/* when the link is very slow, adjust red parameters */
		npkts_per_sec = 1000000 / rp->red_pkttime;
		if (npkts_per_sec < 50) {
			/* up to about 400Kbps */
			rp->red_weight = W_WEIGHT_2;
		} else if (npkts_per_sec < 300) {
			/* up to about 2.4Mbps */
			rp->red_weight = W_WEIGHT_1;
		}
	}

	/* calculate wshift.  weight must be power of 2 */
	w = rp->red_weight;
	for (i = 0; w > 1; i++)
		w = w >> 1;
	rp->red_wshift = i;
	w = 1 << rp->red_wshift;
	if (w != rp->red_weight) {
		printf("invalid weight value %d for red! use %d\n",
		       rp->red_weight, w);
		rp->red_weight = w;
	}

	/*
	 * thmin_s and thmax_s are scaled versions of th_min and th_max
	 * to be compared with avg.
	 */
	rp->red_thmin_s = rp->red_thmin << (rp->red_wshift + FP_SHIFT);
	rp->red_thmax_s = rp->red_thmax << (rp->red_wshift + FP_SHIFT);

	/*
	 * precompute probability denominator
	 *  probd = (2 * (TH_MAX-TH_MIN) / pmax) in fixed-point
	 */
	rp->red_probd = (2 * (rp->red_thmax - rp->red_thmin)
			 * rp->red_inv_pmax) << FP_SHIFT;

	microtime(&rp->red_last);
	return (rp);
}

void
red_destroy(red_t *rp)
{
	free(rp, M_DEVBUF);
}


void
red_getstats(red_t *rp, struct redstats *sp)
{
	sp->q_avg		= rp->red_avg >> rp->red_wshift;
	sp->xmit_cnt		= rp->red_stats.xmit_cnt;
	sp->drop_cnt		= rp->red_stats.drop_cnt;
	sp->drop_forced		= rp->red_stats.drop_forced;
	sp->drop_unforced	= rp->red_stats.drop_unforced;
	sp->marked_packets	= rp->red_stats.marked_packets;
}

#define MHASH_STUB     0x0100007fu

/*
 *  Count packet's destination (or source) address hash
 *  TODOs: 
 *    > hash ipv6 addresses
 */

u_int
hps_pkt_hash(struct mbuf *m, struct altq_pktattr *pktattr, int flags)
{
        struct mbuf     *m0;
        struct ip       *hdr;

        hdr = (struct ip *)m->m_pkthdr.pf.hdr;

        /* verify that hdr is within the mbuf data */
        for (m0 = m; m0 != NULL; m0 = m0->m_next)
                if (((caddr_t)(hdr) >= m0->m_data) &&
                    ((caddr_t)(hdr) < m0->m_data + m0->m_len))
                        break;
        if (m0 == NULL) {
                /* ick, tag info is stale */
                return (MHASH_STUB);
        }

#ifdef HPS_DEBUG
        m0 = m;

        while (m0 != NULL) {
		printf("type=0x%x len=%u flags=0x%x next=%p\n", 
                	m0->m_hdr.mh_type,
                	m0->m_hdr.mh_len,
                	m0->m_hdr.mh_flags,
			m0->m_hdr.mh_next);
                if ((m0->m_hdr.mh_flags & M_PKTHDR) != 0) {
			printf("pf.hdr=%p pf.qid=0x%x\n",
			m0->m_pkthdr.pf.hdr,
			m0->m_pkthdr.pf.qid);
		}

		m0 = m0->m_hdr.mh_next;
	}
#endif

        switch (hdr->ip_v) {
	case 4:	

	/* STUB: just return v4 address as hash */
		if (flags & REDF_ECN)
			return hdr->ip_src.s_addr;
		else
			return hdr->ip_dst.s_addr;
	/* TODO: v6 hash */

	}

/* else - return STUB hash */
        return MHASH_STUB;	
}

/*
 *  Get packet's pre-cached hash
 */

#if 1
#define pkt_hash(m)  ((m)->m_pkthdr.pf.qid)
#else
#define pkt_hash(m)  (hps_pkt_hash(m, NULL, rp->red_flags))
#endif

/*
 *  Enqueue new packet
 */

int
red_addq(red_t *rp, class_queue_t *q, struct mbuf *m,
    struct altq_pktattr *pktattr)
{
        struct    mbuf *m0;  /* tail or head storage */

        struct    mbuf *mi;  /* queue walk index   */
        struct    mbuf *mp;  /* previous index     */
        struct    mbuf *mc;  /* candidate item     */

        u_int     mhash;     /* new packet hash    */
        u_int     ihash;     /* indexed item hash  */
        u_int     phash = 0; /* previous item hash */

        int       qpkts;     /* number of packets (w/ same hash) */
	int       fhead;

        int       n;

        int       qhosts;    /* number of hosts (stub) */

/* count & cache packet hash (hope, qiq is unneeded anymore) */
	m->m_pkthdr.pf.qid = mhash = hps_pkt_hash(m, pktattr, rp->red_flags);

#ifdef HPS_DEBUG
	printf("hash=%x\n", mhash);
#endif

/* shortcut - abort on (global) queue overflow */
	if (qlen(q) >= qlimit(q)) {
		m_freem(m);
		return (-1);
	}

/* shortcut - just add if queue is empty */
        if ((m0 = qtail(q)) == NULL) {
		_addq(q, m);
		return 0;
        }

/* 
 *
 * walk thru queue, in order to:
 * > find a place for new packet
 * > count same-hashed packets to detect personal overflow
 * > ? count number of active hosts
 *
 */

/* prepare to walk */

        mi = m0;              /* current item ptr (set to tail)   */
        mp = NULL;            /* previous item (NULL = n/a)       */
        mc = m;               /* place candidate (self - none, NULL - head) */
        m0  = m0->m_nextpkt;  /* store head ptr (as loop marker)  */
        qpkts  = 0;           /* number of packets with same hash */
        qhosts = 0;           /* number of active hosts detected  */
        fhead  = 0;           /* can put packet on head           */

/* do walk thru queue */
        do {
	/* store previous */
		if (mi->m_nextpkt != m0) {
			mp    = mi;
			phash = ihash;
		}
	/* set on next (or first) item */
		mi    = mi->m_nextpkt;
		ihash = pkt_hash(mi);

	/* hash is equal - skip, count */
		if (ihash == mhash) {
			qpkts++;  // count packets
			continue;
                }

		if (mp == NULL) {
			if (ihash > mhash) fhead = 1;  
		} else {
			if (phash == ihash ||
				(phash > ihash && phash < mhash) || 
				(ihash > mhash && phash < mhash))
				mc = mp;
		}
        } while(mi->m_nextpkt != m0 && mc == m);

        qhosts = 20;  /* STUB */

/* count host queue limit */

#if 0
/* dynamic host queue limit */
        n = qlimit(q)/qhosts;

/* check limit range */
        if (n < 25)
		n = 25;
	else if (n > (qlimit(q)/2))
		n = qlimit(q)/2;

#else
/* fixed host queue limit */
	n = 50;
#endif 

/* check host's limit & drop */
/* (do not allow queue to be overflowed by sigle host) */
        if (qpkts >= n) {
		m_freem(m);
		return (-1);
	}

/* add/insert packet to queue */
	if (mc == m)
		_addq(q, m);                   /* put to tail */
        else {
		if (fhead != 0 && qpkts == 0)
			_insq(q, m, NULL);     /* put to head */
		else
			_insq(q, m, mc);       /* insert to queue */
	}

#ifdef HPS_DEBUG
	mi = qtail(q);
	m0 = qtail(q)->m_nextpkt;

	printf("queue: ");
        do {
                mi = mi->m_nextpkt;
		printf("%x, ", pkt_hash(mi));
	} while(mi->m_nextpkt != m0);	
	printf("\n");
#endif

	return (0);
}

struct mbuf *
red_getq(rp, q)
	red_t *rp;
	class_queue_t *q;
{
	struct mbuf *m;

	if ((m = _getq(q)) == NULL) {
		if (rp->red_idle == 0) {
			rp->red_idle = 1;
			microtime(&rp->red_last);
		}
		return NULL;
	}

	rp->red_idle = 0;
	return (m);
}

