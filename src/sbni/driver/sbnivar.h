/*	$RuOBSD: sbnivar.h,v 1.2 2001/01/16 02:42:20 form Exp $	*/

/*
 * Copyright (c) 2001 Oleg Safiullin
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

#ifndef _DEV_IC_SBNIVAR_H_
#define	_DEV_IC_SBNIVAR_H_

#ifdef _KERNEL
struct sbni_softc {
	struct device		sc_dev;
	struct arpcom		sc_ac;

	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;

	int delta_rxl;

	struct mbuf  *rx_buf_p;		/* receive buffer ptr */
	struct mbuf  *tx_buf_p;		/* transmit buffer ptr */

	u_int pktlen;			/* transmitting pkt length */
	u_int framelen;			/* current frame length */
	u_int maxframe;			/* maximum valid frame length */
	u_int state;
	u_int inppos, outpos;		/* positions in rx/tx buffers */
	u_int tx_frameno;		/* Transmitting frame # */
	u_int wait_frameno;

	u_int trans_errors;
	u_int timer_ticks;

	u_int cur_rxl_index, timeout_rxl;
	u_int32_t cur_rxl_rcvd, prev_rxl_rcvd;

	struct timeout sc_to;
};
#endif	/* _KERNEL */

struct sbni_flags {
#if BYTE_ORDER == BIG_ENDIAN
	u_int8_t		sf_fx_baud:1,
				sf_fx_rxl:1,
				sf_baud:2,
				sf_rxl:4;
	u_int8_t		sf_enaddr[3];
#else
	u_int8_t		sf_enaddr[3];
	u_int8_t		sf_rxl:4,
				sf_baud:2,
				sf_fx_rxl:1,
				sf_fx_baud:1;
#endif
};

struct sbni_info {
	u_int8_t		si_lo_speed;
	u_int8_t		si_baud;
	u_int16_t		si_rxl;
};

#ifdef _KERNEL
void	sbni_attach __P((struct sbni_softc *));
int	sbni_intr __P((void *));
void	sbni_init __P((struct sbni_softc *));
#endif	/* _KERNEL */

#define	SIOCGHWFLAGS	_IOWR('i', 200, struct ifreq)
#define	SIOCSHWFLAGS	_IOWR('i', 201, struct ifreq)
#define SIOCGIFINFO	_IOWR('i', 202, struct ifreq)

#endif /* _DEV_IC_SBNIVAR_H_ */
