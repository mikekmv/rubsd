/* $RuOBSD: pcmaxvar.h,v 1.5 2003/11/25 10:23:48 tm Exp $ */

/*
 * Copyright (c) 2003 Maxim Tsyplakov <tm@openbsd.ru>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DEV_IC_PCMAXVAR_H
#define _DEV_IC_PCMAXVAR_H

#define PCMAX_CAPS	RADIO_CAPS_DETECT_STEREO |	\
			RADIO_CAPS_SET_MONO

struct pcmax_softc {
	struct device		sc_dev;
	bus_space_tag_t		iot;
	bus_space_handle_t	ioh;	

	u_int8_t	ioc;	/* I/O control */
	u_int8_t	iov;	/* I/O value */
	
	int             mute;
	u_int8_t        vol;	
	u_int32_t       freq;
	u_int32_t       stereo;
	u_int32_t       lock;
	
	void	(*set_scl)(struct pcmax_softc *);
	void	(*clr_scl)(struct pcmax_softc *);
	void	(*set_sda)(struct pcmax_softc *);
	void	(*clr_sda)(struct pcmax_softc *);	
};

#endif /* _DEV_IC_PCMAXVAR_H */
