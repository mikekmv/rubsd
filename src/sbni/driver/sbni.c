/*	$RuOBSD: sbni.c,v 1.2 2001/01/16 02:42:19 form Exp $	*/

/*
 * Copyright (c) 2001 Oleg Safiullin
 * Copyright (c) 1997-2000 Granch, Ltd.
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

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/syslog.h>
#include <sys/timeout.h>
#include <sys/proc.h>

#include <net/if.h>
#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <machine/bus.h>

#include <dev/rndvar.h>

#include <dev/ic/sbnireg.h>
#include <dev/ic/sbnivar.h>

extern int hz;

struct cfdriver sbni_cd = {
	NULL, "sbni", DV_IFNET
};

static u_int32_t crc32tab[] = {
	0xd202ef8d,  0xa505df1b,  0x3c0c8ea1,  0x4b0bbe37,
	0xd56f2b94,  0xa2681b02,  0x3b614ab8,  0x4c667a2e,
	0xdcd967bf,  0xabde5729,  0x32d70693,  0x45d03605,
	0xdbb4a3a6,  0xacb39330,  0x35bac28a,  0x42bdf21c,
	0xcfb5ffe9,  0xb8b2cf7f,  0x21bb9ec5,  0x56bcae53,
	0xc8d83bf0,  0xbfdf0b66,  0x26d65adc,  0x51d16a4a,
	0xc16e77db,  0xb669474d,  0x2f6016f7,  0x58672661,
	0xc603b3c2,  0xb1048354,  0x280dd2ee,  0x5f0ae278,
	0xe96ccf45,  0x9e6bffd3,  0x0762ae69,  0x70659eff,
	0xee010b5c,  0x99063bca,  0x000f6a70,  0x77085ae6,
	0xe7b74777,  0x90b077e1,  0x09b9265b,  0x7ebe16cd,
	0xe0da836e,  0x97ddb3f8,  0x0ed4e242,  0x79d3d2d4,
	0xf4dbdf21,  0x83dcefb7,  0x1ad5be0d,  0x6dd28e9b,
	0xf3b61b38,  0x84b12bae,  0x1db87a14,  0x6abf4a82,
	0xfa005713,  0x8d076785,  0x140e363f,  0x630906a9,
	0xfd6d930a,  0x8a6aa39c,  0x1363f226,  0x6464c2b0,
	0xa4deae1d,  0xd3d99e8b,  0x4ad0cf31,  0x3dd7ffa7,
	0xa3b36a04,  0xd4b45a92,  0x4dbd0b28,  0x3aba3bbe,
	0xaa05262f,  0xdd0216b9,  0x440b4703,  0x330c7795,
	0xad68e236,  0xda6fd2a0,  0x4366831a,  0x3461b38c,
	0xb969be79,  0xce6e8eef,  0x5767df55,  0x2060efc3,
	0xbe047a60,  0xc9034af6,  0x500a1b4c,  0x270d2bda,
	0xb7b2364b,  0xc0b506dd,  0x59bc5767,  0x2ebb67f1,
	0xb0dff252,  0xc7d8c2c4,  0x5ed1937e,  0x29d6a3e8,
	0x9fb08ed5,  0xe8b7be43,  0x71beeff9,  0x06b9df6f,
	0x98dd4acc,  0xefda7a5a,  0x76d32be0,  0x01d41b76,
	0x916b06e7,  0xe66c3671,  0x7f6567cb,  0x0862575d,
	0x9606c2fe,  0xe101f268,  0x7808a3d2,  0x0f0f9344,
	0x82079eb1,  0xf500ae27,  0x6c09ff9d,  0x1b0ecf0b,
	0x856a5aa8,  0xf26d6a3e,  0x6b643b84,  0x1c630b12,
	0x8cdc1683,  0xfbdb2615,  0x62d277af,  0x15d54739,
	0x8bb1d29a,  0xfcb6e20c,  0x65bfb3b6,  0x12b88320,
	0x3fba6cad,  0x48bd5c3b,  0xd1b40d81,  0xa6b33d17,
	0x38d7a8b4,  0x4fd09822,  0xd6d9c998,  0xa1def90e,
	0x3161e49f,  0x4666d409,  0xdf6f85b3,  0xa868b525,
	0x360c2086,  0x410b1010,  0xd80241aa,  0xaf05713c,
	0x220d7cc9,  0x550a4c5f,  0xcc031de5,  0xbb042d73,
	0x2560b8d0,  0x52678846,  0xcb6ed9fc,  0xbc69e96a,
	0x2cd6f4fb,  0x5bd1c46d,  0xc2d895d7,  0xb5dfa541,
	0x2bbb30e2,  0x5cbc0074,  0xc5b551ce,  0xb2b26158,
	0x04d44c65,  0x73d37cf3,  0xeada2d49,  0x9ddd1ddf,
	0x03b9887c,  0x74beb8ea,  0xedb7e950,  0x9ab0d9c6,
	0x0a0fc457,  0x7d08f4c1,  0xe401a57b,  0x930695ed,
	0x0d62004e,  0x7a6530d8,  0xe36c6162,  0x946b51f4,
	0x19635c01,  0x6e646c97,  0xf76d3d2d,  0x806a0dbb,
	0x1e0e9818,  0x6909a88e,  0xf000f934,  0x8707c9a2,
	0x17b8d433,  0x60bfe4a5,  0xf9b6b51f,  0x8eb18589,
	0x10d5102a,  0x67d220bc,  0xfedb7106,  0x89dc4190,
	0x49662d3d,  0x3e611dab,  0xa7684c11,  0xd06f7c87,
	0x4e0be924,  0x390cd9b2,  0xa0058808,  0xd702b89e,
	0x47bda50f,  0x30ba9599,  0xa9b3c423,  0xdeb4f4b5,
	0x40d06116,  0x37d75180,  0xaede003a,  0xd9d930ac,
	0x54d13d59,  0x23d60dcf,  0xbadf5c75,  0xcdd86ce3,
	0x53bcf940,  0x24bbc9d6,  0xbdb2986c,  0xcab5a8fa,
	0x5a0ab56b,  0x2d0d85fd,  0xb404d447,  0xc303e4d1,
	0x5d677172,  0x2a6041e4,  0xb369105e,  0xc46e20c8,
	0x72080df5,  0x050f3d63,  0x9c066cd9,  0xeb015c4f,
	0x7565c9ec,  0x0262f97a,  0x9b6ba8c0,  0xec6c9856,
	0x7cd385c7,  0x0bd4b551,  0x92dde4eb,  0xe5dad47d,
	0x7bbe41de,  0x0cb97148,  0x95b020f2,  0xe2b71064,
	0x6fbf1d91,  0x18b82d07,  0x81b17cbd,  0xf6b64c2b,
	0x68d2d988,  0x1fd5e91e,  0x86dcb8a4,  0xf1db8832,
	0x616495a3,  0x1663a535,  0x8f6af48f,  0xf86dc419,
	0x660951ba,  0x110e612c,  0x88073096,  0xff000000
};

static u_char rxl_tab[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
	0x0a, 0x0c, 0x0f, 0x16, 0x18, 0x1a, 0x1c, 0x1f
};

static u_char timeout_rxl_tab[] = {
	0x03, 0x05, 0x08, 0x0b
};

static void	sbni_start __P((struct ifnet *));
static void	sbni_stop __P((struct sbni_softc *));
static int	sbni_ioctl __P((struct ifnet *, u_long, caddr_t));
static void	sbni_reset __P((struct sbni_softc *));
static void	sbni_watchdog __P((struct ifnet *));
static void	sbni_timeout __P((void *));
static void	prepare_to_send __P((struct sbni_softc *));
static void	drop_xmit_queue __P((struct sbni_softc *));
static int	recv_frame __P((struct sbni_softc *));
static void	send_frame __P((struct sbni_softc *));
static int	check_fhdr __P((struct sbni_softc *, u_int *, u_int *, u_int *,
    u_int *, u_int32_t *)); 
static int	upload_data __P((struct sbni_softc *, u_int, u_int, u_int,
    u_int32_t));
static int	skip_tail __P((struct sbni_softc *, u_int, u_int32_t));
static void	interpret_ack __P((struct sbni_softc *, u_int));
static void	send_frame_header __P((struct sbni_softc *, u_int32_t *));
static void	download_data __P((struct sbni_softc *, u_int32_t *));
static u_int32_t calc_crc32 __P((u_int32_t, caddr_t, u_int));
static int	append_frame_to_pkt __P((struct sbni_softc *, u_int, u_int32_t));
static int	get_rx_buf __P((struct sbni_softc *));
static void	indicate_pkt __P((struct sbni_softc *));
static void	timeout_change_level __P((struct sbni_softc *));
static void	set_sbni_flags __P((struct sbni_softc *, u_int32_t));
static void	set_hardware_flags __P((struct sbni_softc *));
static void	change_level __P((struct sbni_softc *));
static void	set_csr1 __P((struct sbni_softc *, int));

void
sbni_attach(sc)
	struct sbni_softc *sc;
{
	struct sbni_flags *sf = (void *)&sc->sc_dev.dv_cfdata->cf_flags;
	struct ifnet *ifp = &sc->sc_ac.ac_if;

	sc->sc_ac.ac_enaddr[0] = 0x00;
	sc->sc_ac.ac_enaddr[1] = 0xff;
	sc->sc_ac.ac_enaddr[2] = 0x01;
	if ((sf->sf_enaddr[0] | sf->sf_enaddr[1] | sf->sf_enaddr[2]) == 0)
		get_random_bytes(&sf->sf_enaddr, sizeof(sf->sf_enaddr));
	set_hardware_flags(sc);

	sc->maxframe = DEFAULT_FRAME_LEN;
	sc->timeout_rxl = 0;

	ifp->if_softc = sc;
	bcopy(sc->sc_dev.dv_xname, ifp->if_xname, IFNAMSIZ);
	ifp->if_output = ether_output;
	ifp->if_start = sbni_start;
	ifp->if_ioctl = sbni_ioctl;
	ifp->if_watchdog = sbni_watchdog;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags = IFF_SIMPLEX | IFF_BROADCAST | IFF_MULTICAST;

	if_attach(ifp);
	ether_ifattach(ifp);
#if NBPFILTER > 0
	bpfattach(&ifp->if_bpf, ifp, DLT_EN10MB, sizeof(struct ether_header));
#endif

	timeout_set(&sc->sc_to, sbni_timeout, sc);
}

void 
sbni_init(sc)
	struct sbni_softc *sc;
{
	struct ifnet *ifp = &sc->sc_ac.ac_if;
	int s;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	s = splimp();

	ifp->if_timer = 0;
	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;

	sc->timer_ticks = CHANGE_LEVEL_START_TICKS;
	sc->state &= ~(FL_WAIT_ACK | FL_NEED_RESEND);
	sc->state |= FL_PREV_OK;
	sc->inppos = sc->wait_frameno = 0;

	set_csr1(sc, 1);
	OUTB(CSR0, EN_INT);

	timeout_add(&sc->sc_to, hz / SBNI_HZ);

	sbni_start(ifp);

	splx(s);
}

static void 
sbni_start(ifp)
	struct ifnet *ifp;
{
	struct sbni_softc *sc = ifp->if_softc;

	if (sc->tx_frameno == 0)
		prepare_to_send(sc);
}

static void 
sbni_stop(sc)
	struct sbni_softc *sc;
{
	OUTB(CSR0, 0);

	drop_xmit_queue(sc);
	if (sc->rx_buf_p != NULL) {
		m_freem(sc->rx_buf_p);
		sc->rx_buf_p = NULL;
	}
	timeout_del(&sc->sc_to);
}

int
sbni_intr(arg)
	void *arg;
{
	struct sbni_softc *sc = arg;

	int req_ans;
	u_int8_t csr0;

	OUTB(CSR0, (INB(CSR0) & ~EN_INT) | TR_REQ);

	sc->timer_ticks = CHANGE_LEVEL_START_TICKS;
	for(;;) {
		csr0 = INB(CSR0);
		if ((csr0 & (RC_RDY | TR_RDY)) == 0)
			break;

		req_ans = !(sc->state & FL_PREV_OK);

		if (csr0 & RC_RDY)
			req_ans = recv_frame(sc);

		/*
		 * TR_RDY always equals 1 here because we own the marker,
		 * and have setted TR_REQ when interrupts have disabled
		 */
		csr0 = INB(CSR0);
		if (!(csr0 & TR_RDY) || (csr0 & RC_RDY))
			printf("%s: internal error\n",
			    sc->sc_ac.ac_if.if_xname);

		/* if state & FL_NEED_RESEND != 0 then tx_frameno != 0 */
		if (req_ans || sc->tx_frameno != 0)
			send_frame(sc);
		else
			/* send marker without any data */
			OUTB(CSR0, INB(CSR0) & ~TR_REQ);
	}

	OUTB(CSR0, INB(CSR0) | EN_INT);
	return (1);
}

void 
sbni_watchdog(ifp)
	struct ifnet *ifp;
{
	struct sbni_softc *sc = ifp->if_softc;

	log(LOG_ERR, "%s: device timeout\n", ifp->if_xname);
	++sc->sc_ac.ac_if.if_oerrors;

	sbni_reset(sc);
}

static void
sbni_timeout(xsc)
	void *xsc;
{
	struct sbni_softc *sc = (struct sbni_softc *)xsc;
	int s;
	u_int8_t csr0;

	s = splimp();

	csr0 = INB(CSR0);
	if (csr0 & RC_CHK) {
		if (sc->timer_ticks) {
			if (csr0 & (RC_RDY | BU_EMP))
				/* receiving not active */
				sc->timer_ticks--;
		} else {
			if (sc->delta_rxl)
				timeout_change_level(sc);

			set_csr1(sc, 1);
			csr0 = INB(CSR0);
		}
	}

	OUTB(CSR0, csr0 | RC_CHK); 
	timeout_add(&sc->sc_to, hz/SBNI_HZ);
	splx(s);
}

static int
sbni_ioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	u_long cmd;
	caddr_t data;
{
	struct ifaddr *ifa = (struct ifaddr *)data;
	struct sbni_softc *sc = ifp->if_softc;
	struct ifreq *ifr = (struct ifreq *) data;
	struct proc *p = curproc;
	int s, error = 0;

	s = splimp();

	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		switch(ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			sbni_init(sc);
			arp_ifinit((struct arpcom *)ifp, ifa);
			break;
#endif
		default:
			sbni_init(sc);
			break;
		}
		break;
	case SIOCSIFFLAGS:
		if ((ifp->if_flags & IFF_UP) == 0 &&
		    ifp->if_flags & IFF_RUNNING) {
			ifp->if_flags &= ~IFF_RUNNING;
			sbni_stop(sc);
		} else
			sbni_init(sc);
		break;
	case SIOCGHWFLAGS:
		ifr->ifr_data = *(caddr_t *)&sc->sc_dev.dv_cfdata->cf_flags;
		break;
	case SIOCSHWFLAGS:
		/* Only root can do this */
		if ((error = suser(p->p_ucred, &p->p_acflag)))
			break;

		set_sbni_flags(sc, *(u_int32_t *)&ifr->ifr_data);
		set_hardware_flags(sc);
		break;
	case SIOCGIFINFO:
		{
			struct sbni_info si;

			si.si_lo_speed = INB(CSR0) & 0x01;
			si.si_baud = ifp->if_baudrate / 62500;
			si.si_rxl = sc->cur_rxl_index;
			ifr->ifr_data = *(caddr_t *)&si;
		}
		break;
	default:
		error = EINVAL;
		break;
	}

	splx(s);

	return (error);
}

static void 
sbni_reset(sc)
	struct sbni_softc *sc;
{
	int s;

	s = splimp();
	sbni_stop(sc);
	sbni_init(sc);
	splx(s);
}

static void
prepare_to_send(sc)
	struct sbni_softc  *sc;
{
	struct mbuf *m;
	u_int len;

	if (sc->tx_buf_p != NULL)
		printf("%s: memory leak detected\n",
		    sc->sc_ac.ac_if.if_xname);

	sc->outpos = 0;
	sc->state &= ~(FL_WAIT_ACK | FL_NEED_RESEND);

	for (;;) {
		IF_DEQUEUE(&sc->sc_ac.ac_if.if_snd, sc->tx_buf_p);
		if (sc->tx_buf_p == NULL) {
			/* nothing to transmit... */
			sc->pktlen = sc->tx_frameno = sc->framelen = 0;
			sc->sc_ac.ac_if.if_flags &= ~IFF_OACTIVE;
			return;
		}

		for (len = 0, m = sc->tx_buf_p; m != NULL; m = m->m_next)
			len += m->m_len;

		if (len != 0)
			break;
		m_freem(sc->tx_buf_p);
	}

	if (len < SBNI_MIN_LEN)
		len = SBNI_MIN_LEN;

	sc->pktlen = len;
	sc->tx_frameno = (len + sc->maxframe - 1) / sc->maxframe;
	sc->framelen = min(len, sc->maxframe);

	OUTB(CSR0, INB(CSR0) | TR_REQ);
	sc->sc_ac.ac_if.if_flags |= IFF_OACTIVE;
#if NBPFILTER > 0
	if (sc->sc_ac.ac_if.if_bpf)
		bpf_mtap(sc->sc_ac.ac_if.if_bpf, sc->tx_buf_p);
#endif
}

static void
drop_xmit_queue(sc)
	struct sbni_softc *sc;
{
	struct mbuf *m;

	if (sc->tx_buf_p != NULL) {
		m_freem(sc->tx_buf_p);
		sc->tx_buf_p = NULL;
		sc->sc_ac.ac_if.if_oerrors++;
	}

	for (;;) {
		IF_DEQUEUE(&sc->sc_ac.ac_if.if_snd, m);
		if(m == NULL)
			break;
		m_freem(m);
		sc->sc_ac.ac_if.if_oerrors++;
	}

	sc->tx_frameno = sc->framelen = sc->outpos = 0;
	sc->state &= ~(FL_WAIT_ACK | FL_NEED_RESEND);
	sc->sc_ac.ac_if.if_flags &= ~IFF_OACTIVE;
}

static int
recv_frame(sc)
	struct sbni_softc  *sc;
{
	u_int32_t crc = CRC32_INITIAL;
	u_int framelen, frameno, ack;
	u_int is_first, frame_ok;

	if (check_fhdr(sc, &framelen, &frameno, &ack, &is_first, &crc)) {
		frame_ok = framelen > 4
		    ? upload_data( sc, framelen, frameno, is_first, crc)
		    : skip_tail(sc, framelen, crc);
		if (frame_ok) {
			interpret_ack(sc, ack);
		}
	} else
		frame_ok = 0;

	OUTB(CSR0, INB(CSR0) ^ CT_ZER);
	if (frame_ok) {
		sc->state |= FL_PREV_OK;
	} else {
		sc->state &= ~FL_PREV_OK;
		change_level(sc);
	}

	return  !frame_ok  ||  framelen > 4;
}


static void
send_frame(sc)
	struct sbni_softc *sc;
{
	u_int32_t crc = CRC32_INITIAL;
	u_char csr0;

	if (sc->state & FL_NEED_RESEND) {
		/* if frame was sended but not ACK'ed - resend it */
		if (sc->trans_errors) {
			sc->trans_errors--;
		} else {
			/* cannot xmit with many attempts */
			drop_xmit_queue(sc);
			goto do_send;
		}
	} else
		sc->trans_errors = TR_ERROR_COUNT;

	send_frame_header(sc, &crc);

	/*
	 * FL_NEED_RESEND will be cleared after ACK, but if empty
	 * frame sended then in prepare_to_send next frame
	 */
	sc->state |= FL_NEED_RESEND;

	if (sc->framelen) {
		download_data(sc, &crc);
		sc->state |= FL_WAIT_ACK;
	}
	OUTSB(DAT, (u_char *)&crc, sizeof(crc));

do_send:
	csr0 = INB(CSR0);
	OUTB(CSR0, csr0 & ~TR_REQ);

	if (sc->tx_frameno)
		/* next frame exists - request to send */
		OUTB(CSR0, csr0 | TR_REQ);
}

static int
check_fhdr(sc, framelen, frameno, ack, is_first, crc_p)
	struct sbni_softc *sc;
	u_int *framelen;
	u_int *frameno;
	u_int *ack;
	u_int *is_first;
	u_int32_t *crc_p;
{
	u_int32_t crc = *crc_p;
	u_char value;

	if ((value = INB(DAT)) != SBNI_SIG)
		return (0);

	value = INB(DAT);
	*framelen = (u_int)value;
	crc = CRC32(value, crc);
	value = INB(DAT);
	*framelen |= ((u_int)value) << 8;
	crc = CRC32(value, crc);

	*ack = *framelen & FRAME_ACK_MASK;
	*is_first = (*framelen & FRAME_FIRST) != 0;

	if ((*framelen &= FRAME_LEN_MASK) < 6
	    ||  *framelen > SBNI_MAX_FRAME - 3)
		return (0);

	value = INB(DAT);
	*frameno = (u_int)value;
	crc = CRC32(value, crc);

	/* reserved byte */
	crc = CRC32(INB(DAT), crc);
	*framelen -= 2;

	*crc_p = crc;
	return (1);
}

static int
upload_data(sc, framelen, frameno, is_first, crc)
	struct sbni_softc *sc;
	u_int framelen;
	u_int frameno;
	u_int is_first;
	u_int32_t crc;
{
	int frame_ok;
	if (is_first)
		sc->wait_frameno = frameno,
		sc->inppos = 0;

	if (sc->wait_frameno == frameno) {
		if (sc->inppos + framelen  <=  ETHER_MAX_LEN)
			frame_ok = append_frame_to_pkt(sc, framelen, crc);

		/*
		 * if CRC is right but framelen incorrect then transmitter
		 * error was occured... drop entire packet
		 */
		else
			if ((frame_ok = skip_tail(sc, framelen, crc)) != 0) {
				sc->wait_frameno = sc->inppos = 0;
				sc->sc_ac.ac_if.if_ierrors++;
			}
	} else
		frame_ok = skip_tail(sc, framelen, crc);

	if (is_first && !frame_ok) {
		/*
		 * Frame has been violated, but we have stored
		 * is_first already... Drop entire packet.
		 */
		sc->wait_frameno = 0;
		sc->sc_ac.ac_if.if_ierrors++;
	}

	return (frame_ok);
}

static int
skip_tail(sc, tail_len, crc)
	struct sbni_softc *sc;
	u_int tail_len;
	u_int32_t crc;
{
	while (tail_len--)
		crc = CRC32(INB(DAT), crc);

	return (crc == CRC32_REMAINDER);
}

static void
interpret_ack(sc, ack)
	struct sbni_softc *sc;
	u_int ack;
{
	if (ack == FRAME_SENT_OK) {
		sc->state &= ~FL_NEED_RESEND;

		if (sc->state & FL_WAIT_ACK) {
			sc->outpos += sc->framelen;

			if (--sc->tx_frameno)
				sc->framelen = min(sc->maxframe,
				    sc->pktlen - sc->outpos);
			else {
				m_freem(sc->tx_buf_p);
				sc->tx_buf_p = NULL;
				sc->sc_ac.ac_if.if_opackets++;
				prepare_to_send(sc);
			}
		}
	}

	sc->state &= ~FL_WAIT_ACK;
}

static void
send_frame_header(sc, crc_p)
	struct sbni_softc *sc;
	u_int32_t *crc_p;
{
	u_int32_t crc = *crc_p;
	u_int len_field = sc->framelen + 6;	/* CRC + frameno + reserved */
	u_char value;

	if (sc->state & FL_NEED_RESEND)
		len_field |= FRAME_RETRY;	/* non-first attempt... */

	if (sc->outpos == 0)
		len_field |= FRAME_FIRST;

	len_field |= (sc->state & FL_PREV_OK) ? FRAME_SENT_OK : FRAME_SENT_BAD;
	OUTB(DAT, SBNI_SIG);

	value = (u_char)len_field;
	OUTB(DAT, value);
	crc = CRC32(value, crc);
	value = (u_char)(len_field >> 8);
	OUTB(DAT, value);
	crc = CRC32(value, crc);

	OUTB(DAT, sc->tx_frameno);
	crc = CRC32(sc->tx_frameno, crc);
	OUTB(DAT, 0);
	crc = CRC32(0, crc);
	*crc_p = crc;
}

static void
download_data(sc, crc_p)
	struct sbni_softc *sc;
	u_int32_t *crc_p;
{
	struct mbuf *m = sc->tx_buf_p;
	caddr_t	data_p;
	u_int data_len;
	u_int pos = 0;

	while(m && pos < sc->pktlen) {
		if (pos + m->m_len  >  sc->outpos) {
			data_len = m->m_len - (sc->outpos - pos);
			data_p = mtod(m, caddr_t) + (sc->outpos - pos);

			goto do_copy;
		} else
			pos += m->m_len;
		m = m->m_next;
	}

	data_len = 0;

do_copy:
	pos = 0;
	do
		if (data_len) {
			u_int slice = min(data_len, sc->framelen - pos);
			OUTSB(DAT, data_p, slice);
			*crc_p = calc_crc32(*crc_p, data_p, slice);

			pos += slice;
			if (data_len -= slice)
				data_p += slice;
			else {
				do
					m = m->m_next;
				while (m && m->m_len == 0);
				if (m != NULL) {
					data_len = m->m_len;
					data_p = mtod(m, caddr_t);
				}
			}
		} else {
			/* frame too short - zero padding */

			pos = sc->framelen - pos;
			while (pos--) {
				OUTB(DAT, 0);
				*crc_p = CRC32(0, *crc_p);
			}
			return;
		}
	while (pos < sc->framelen);
}

static u_int32_t
calc_crc32(crc, p, len)
	u_int32_t crc;
	caddr_t p;
	u_int len;
{
	while (len--)
		crc = CRC32(*p++, crc);

	return (crc);
}

static int
append_frame_to_pkt(sc, framelen, crc)
	struct sbni_softc *sc;
	u_int framelen;
	u_int32_t crc;
{
	caddr_t p;

	if (sc->inppos + framelen > ETHER_MAX_LEN)
		return (0);

	if (!sc->rx_buf_p && !get_rx_buf(sc))
		return (0);

	p = sc->rx_buf_p->m_data + sc->inppos;
	INSB(DAT, p, framelen);
	if (calc_crc32(crc, p, framelen) != CRC32_REMAINDER)
		return (0);

	sc->inppos += framelen - 4;
	if (--sc->wait_frameno == 0) {
		/* last frame received */
		indicate_pkt(sc);
		sc->sc_ac.ac_if.if_ipackets++;
	}

	return (1);
}

static int
get_rx_buf(sc)
	struct sbni_softc *sc;
{
	struct mbuf *m;

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == NULL) {
		printf("%s: cannot allocate header mbuf\n",
		    sc->sc_ac.ac_if.if_xname);
		return (0);
	}

	/*
	 * We always put the received packet in a single buffer -
	 * either with just an mbuf header or in a cluster attached
	 * to the header. The +2 is to compensate for the alignment
	 * fixup below.
	 */
	if (ETHER_MAX_LEN + 2 > MHLEN) {
		/* Attach an mbuf cluster */
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
			m_freem(m);
			return (0);
		}
	}
	m->m_pkthdr.len = m->m_len = ETHER_MAX_LEN + 2;

	/*
	 * The +2 is to longword align the start of the real packet.
	 * (sizeof ether_header == 14)
	 * This is important for NFS.
	 */
	m_adj(m, 2);
	sc->rx_buf_p = m;
	return (1);
}

static void
indicate_pkt(sc)
	struct sbni_softc *sc;
{
	struct mbuf *m = sc->rx_buf_p;
	struct ether_header *eh;

	m->m_pkthdr.rcvif = &sc->sc_ac.ac_if;
	eh = mtod(m, struct ether_header *);

#if NBPFILTER > 0
	/*
	 * Check if there's a BPF listener on this interface. If so, hand off
	 * the raw packet to bpf.
	 */
	if (sc->sc_ac.ac_if.if_bpf)
		bpf_mtap(sc->sc_ac.ac_if.if_bpf, m);
#endif

	/*
	 * Remove link layer address and indicate packet.
	 * NB: bpf_mtap will be called by ether_input in FreeBSD 4.x
	 */
	m->m_pkthdr.len = m->m_len = sc->inppos;
	m_adj(m, sizeof(struct ether_header));

	ether_input(&sc->sc_ac.ac_if, eh, m);
	sc->rx_buf_p = NULL;
}

static void
timeout_change_level(sc)
	struct sbni_softc *sc;
{
	sc->cur_rxl_index = timeout_rxl_tab[sc->timeout_rxl];
	if (++sc->timeout_rxl >= sizeof(timeout_rxl_tab))
		sc->timeout_rxl = 0;

	set_csr1(sc, 0);
	sc->prev_rxl_rcvd = sc->cur_rxl_rcvd;
	sc->cur_rxl_rcvd = 0;
}

static void
set_sbni_flags(sc, flags)
	struct sbni_softc *sc;
	u_int32_t flags;
{
	struct sbni_flags *sf = (void *)&sc->sc_dev.dv_cfdata->cf_flags;
	struct sbni_flags *nsf = (void *)&flags;

	if ((nsf->sf_enaddr[0] | nsf->sf_enaddr[1] | nsf->sf_enaddr[2]) == 0)
		bcopy(&sf->sf_enaddr, nsf->sf_enaddr, sizeof(nsf->sf_enaddr));
	bcopy(nsf, sf, sizeof(sf));
}

static void
set_hardware_flags(sc)
	struct sbni_softc *sc;
{
	struct sbni_flags *sf = (void *)&sc->sc_dev.dv_cfdata->cf_flags;

	/* Set MAC address */
#if BYTE_ORDER == BIG_ENDIAN
	sc->sc_ac.ac_enaddr[3] = sf->sf_enaddr[0];
	sc->sc_ac.ac_enaddr[4] = sf->sf_enaddr[1];
	sc->sc_ac.ac_enaddr[5] = sf->sf_enaddr[2];
#else
	sc->sc_ac.ac_enaddr[3] = sf->sf_enaddr[2];
	sc->sc_ac.ac_enaddr[4] = sf->sf_enaddr[1];
	sc->sc_ac.ac_enaddr[5] = sf->sf_enaddr[0];
#endif

	if (sf->sf_fx_rxl) {
		sc->delta_rxl = 0;
		sc->cur_rxl_index = sf->sf_rxl;
	} else {
		sc->delta_rxl = DEF_RXL_DELTA;
		sc->cur_rxl_index = DEF_RXL;
	}
	sc->sc_ac.ac_if.if_baudrate =
	    (INB(CSR0) & 0x01 ? 500000 : 2000000) / (1 << sf->sf_baud);

	printf("%s: address %s, speed %d%s kbit/s, receive level ",
	    sc->sc_dev.dv_xname, ether_sprintf(sc->sc_ac.ac_enaddr),
	    sc->sc_ac.ac_if.if_baudrate / 1000,
	    sc->sc_ac.ac_if.if_baudrate % 1000 ? ".5" : "");
	if (sf->sf_fx_rxl)
		printf("%d\n", sf->sf_rxl);
	else
		printf("auto\n");

	set_csr1(sc, 0);
}

static void
change_level(sc)
	struct sbni_softc *sc;
{
	if (sc->delta_rxl == 0)
		return;

	if (sc->cur_rxl_index == 0)
		sc->delta_rxl = 1;
	else if (sc->cur_rxl_index == 15)
		sc->delta_rxl = -1;
	else if (sc->cur_rxl_rcvd < sc->prev_rxl_rcvd)
		sc->delta_rxl = -sc->delta_rxl;

	set_csr1(sc, 0);
	sc->prev_rxl_rcvd = sc->cur_rxl_rcvd;
	sc->cur_rxl_rcvd  = 0;
}

static void
set_csr1(sc, res)
	struct sbni_softc *sc;
	int res;
{
	struct sbni_flags *sf = (void *)&sc->sc_dev.dv_cfdata->cf_flags;

	if (res)
		OUTB(CSR1, rxl_tab[sc->cur_rxl_index] | (sf->sf_baud << 5) |
		    PR_RES);
	else
		OUTB(CSR1, rxl_tab[sc->cur_rxl_index] | (sf->sf_baud << 5));
}
