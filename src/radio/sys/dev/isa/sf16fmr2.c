/* $RuOBSD: sf16fmr2.c,v 1.3 2001/09/29 14:29:38 pva Exp $ */

/*
 * Copyright (c) 2001 Maxim Tsyplakov <tm@oganer.net>,
 *                    Vladimir Popov <jumbo@narod.ru>
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

/* SoundForte RadioLink SF16-FMR2 FM Radio Card device driver */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/radioio.h>

#include <dev/isa/isavar.h>
#include <dev/radio_if.h>
#include <dev/tea5757.h>

#define SF16FMR2_BASE_VALID(x)	(x == 0x384)
#define SF16FMR2_CAPABILITIES	\
			RADIO_CAPS_DETECT_STEREO | \
			RADIO_CAPS_DETECT_SIGNAL | \
			RADIO_CAPS_SET_MONO | \
			RADIO_CAPS_LOCK_SENSITIVITY | \
			RADIO_CAPS_HW_AFC | \
			RADIO_CAPS_HW_SEARCH

int    sf2r_probe(struct device *, void *, void *);
void   sf2r_attach(struct device *, struct device * self, void *);
int    sf2r_open(dev_t, int, int, struct proc *);
int    sf2r_close(dev_t, int, int, struct proc *);
int    sf2r_ioctl(dev_t, u_long, caddr_t, int, struct proc *);

/* define our interface to the higher level radio driver */
struct radio_hw_if sf2r_hw_if = {
	sf2r_open,
	sf2r_close,
	sf2r_ioctl
};

struct sf2r_softc {
	struct device   sc_dev;
	bus_space_tag_t sc_iot;
	bus_space_handle_t sc_ioh;

	u_long          sc_freq;
	u_char          sc_vol;
	u_char          sc_mute;
	u_long          sc_stereo;
	u_long          sc_lock;
};

struct cfattach sf2r_ca = {
	sizeof(struct sf2r_softc), sf2r_probe, sf2r_attach
};

struct cfdriver sf2r_cd = {
	NULL, "sf2r", DV_DULL
};

u_int   sf2r_find(bus_space_tag_t, bus_space_handle_t);
void    sf2r_reset(struct sf2r_softc *);

void    sf2r_set_mute(struct sf2r_softc *);
void    sf2r_set_freq(struct sf2r_softc *, u_long);
u_int   sf2r_state(bus_space_tag_t, bus_space_handle_t);
void    sf2r_search(struct sf2r_softc *, u_char);

void    sf2r_write_shift_register(struct sf2r_softc *, u_long);
u_long  sf2r_read_shift_register(bus_space_tag_t, bus_space_handle_t);

static u_long   tea5757_decode_freq(u_long);

int
sf2r_probe(struct device  *parent, void *self, void *aux)
{
	struct isa_attach_args *ia = aux;
	bus_space_tag_t iot = ia->ia_iot;
	bus_space_handle_t ioh;

	int iosize = 1, iobase = ia->ia_iobase;

	if (!SF16FMR2_BASE_VALID(iobase)) {
		printf("sf2r: configured iobase 0x%x invalid\n", iobase);
		return 0;
	}

	if (bus_space_map(iot, iobase, iosize, 0, &ioh))
		return 0;

	bus_space_unmap(iot, ioh, iosize);

	if (!sf2r_find(iot, ioh))
		return 0;

	ia->ia_iosize = iosize;
	return 1;
}

void
sf2r_attach(struct device *parent, struct device *self, void *aux)
{
	struct sf2r_softc *sc = (void *) self;
	struct isa_attach_args *ia = aux;

	sc->sc_iot = ia->ia_iot;
#ifdef RADIO_INIT_MUTE
	sc->sc_mute = RADIO_INIT_MUTE;
#else
	sc->sc_mute = 1;
#endif /* RADIO_INIT_MUTE */
#ifdef RADIO_INIT_VOL
	sc->sc_vol = RADIO_INIT_VOL;
#else
	sc->sc_vol = 0;
#endif /* RADIO_INIT_VOL */
#ifdef RADIO_INIT_FREQ
	sc->sc_freq = RADIO_INIT_FREQ;
#else
	sc->sc_freq = MIN_FM_FREQ;
#endif /* RADIO_INIT_FREQ */
	sc->sc_stereo = TEA5757_STEREO;
	sc->sc_lock = TEA5757_S030;

	/* remap I/O */
	if (bus_space_map(sc->sc_iot, ia->ia_iobase, ia->ia_iosize,
			  0, &sc->sc_ioh))
		panic("sf2rattach: bus_space_map() failed");

	printf(": SoundForte RadioLink SF16-FMR2");
	sf2r_reset(sc);

	radio_attach_mi(&sf2r_hw_if, sc, &sc->sc_dev);
}

int
sf2r_open(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct sf2r_softc *sc;
	return  !(sc = sf2r_cd.cd_devs[0]) ? ENXIO : 0;
}

int
sf2r_close(dev_t dev, int flags, int fmt, struct proc *p)
{
	return 0;
}

/*
 * Handle the ioctl for the device
 */
int
sf2r_ioctl(dev_t dev, u_long cmd, caddr_t data, int flags, struct proc *p)
{
	struct sf2r_softc *sc = sf2r_cd.cd_devs[0];
	int error;

	error = 0;
	switch (cmd) {
	case RIOCGMUTE:
		*(u_long *)data = sc->sc_mute ? 1 : 0;
		break;
	case RIOCSMUTE:
		sc->sc_mute = *(u_long *)data ? 1 : 0;
		sf2r_set_mute(sc);
		break;
	case RIOCGVOLU:
		*(u_long *)data = sc->sc_vol ? 255 : 0;
		break;
	case RIOCSVOLU:
		sc->sc_vol = *(u_long *)data ? 1 : 0;
		sf2r_set_mute(sc);
		break;
	case RIOCGMONO:
		if (sc->sc_stereo == TEA5757_STEREO)
			*(u_long *)data = 0;
		else
			*(u_long *)data = 1;
		break;
	case RIOCSMONO:
		sc->sc_stereo = *(u_long *)data ? TEA5757_MONO : TEA5757_STEREO;
		sf2r_set_freq(sc, sc->sc_freq);
		break;
	case RIOCGFREQ:
		sc->sc_freq =
			tea5757_decode_freq(sf2r_read_shift_register(sc->sc_iot, sc->sc_ioh));
		*(u_long *)data = sc->sc_freq;
		break;
	case RIOCSFREQ:
		sf2r_set_freq(sc, *(u_long *)data);
		break;
	case RIOCSSRCH:
		sf2r_search(sc, *(u_long *)data);
		break;
	case RIOCGCAPS:
		*(u_long *)data = SF16FMR2_CAPABILITIES;
		break;
	case RIOCGINFO:
		*(u_long *)data = sf2r_state(sc->sc_iot, sc->sc_ioh);
		break;
	case RIOCSREFF:
	case RIOCGREFF:
		error = ENODEV;
		break;
	case RIOCSLOCK:
		if (*(u_long *)data < 8)
			sc->sc_lock = TEA5757_S005;
		else if (*(u_long *)data > 7 && *(u_long *)data < 15)
			sc->sc_lock = TEA5757_S010;
		else if (*(u_long *)data > 14 && *(u_long *)data < 51)
			sc->sc_lock = TEA5757_S030;
		else if (*(u_long *)data > 50)
			sc->sc_lock = TEA5757_S150;
		sf2r_set_freq(sc, sc->sc_freq);
		break;
	case RIOCGLOCK:
		switch (sc->sc_lock) {
		case TEA5757_S005:
			*(u_long *)data = 5;
			break;
		case TEA5757_S010:
			*(u_long *)data = 10;
			break;
		case TEA5757_S150:
			*(u_long *)data = 150;
			break;
		case TEA5757_S030:
		default:
			*(u_long *)data = 30;
			break;
		}
		break;
	default:
		error = EINVAL;	/* invalid agument */
	}
	return error;
}

/*
 * Mute the card
 */
void
sf2r_set_mute(struct sf2r_softc *sc)
{
	u_char mute = sc->sc_mute || !sc->sc_vol;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, mute ? 0x00 : 0x04);
	DELAY(6);
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, mute ? 0x00 : 0x04);
}

void
sf2r_write_shift_register(struct sf2r_softc *sc, u_long data)
{
	int i = 25;

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x00);

	while (i--)
		if (data & (1<<i)) {
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x01);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x03);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x01);
		} else {
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x00);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x02);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x00);
		}

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x00);
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, 0x04);

	sf2r_set_mute(sc);
}

void
sf2r_reset(struct sf2r_softc *sc)
{
	sf2r_set_freq(sc, sc->sc_freq);
	sf2r_set_mute(sc);
}


u_int
sf2r_find(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	struct sf2r_softc sc;
	u_long freq;

	sc.sc_iot = iot;
	sc.sc_ioh = ioh;
	sc.sc_lock = TEA5757_S030;
	sc.sc_stereo = TEA5757_STEREO;

	if ((bus_space_read_1(iot, ioh, 0) & 0x70) == 0x30) {
		/*
		 * Let's try to write and read a frequency.
		 * If the written and read frequencies are the same then success
		 */
#ifdef RADIO_INIT_FREQ
		sc.sc_freq = RADIO_INIT_FREQ;
#else
		sc.sc_freq = MIN_FM_FREQ;
#endif /* RADIO_INIT_FREQ */
		sf2r_set_freq(&sc, sc.sc_freq);
		freq = sf2r_read_shift_register(sc.sc_iot, sc.sc_ioh);
		if (tea5757_decode_freq(freq) == sc.sc_freq)
			return 1;
	}

	return 0;
}

void
sf2r_set_freq(struct sf2r_softc *sc, u_long nfreq)
{
	u_long data = 0ul;

	if (nfreq < MIN_FM_FREQ)
		nfreq = MIN_FM_FREQ;
	if (nfreq > MAX_FM_FREQ)
		nfreq = MAX_FM_FREQ;

	sc->sc_freq = nfreq;

	nfreq += IF_FREQ;
	/*
	 * NO FLOATING POINT!
	 */
	nfreq *= 10;
	nfreq /= 125;

	data = nfreq | TEA5757_SEARCH_END | sc->sc_stereo | sc->sc_lock;
	sf2r_write_shift_register(sc, data);

	return;
}

u_int
sf2r_state(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	u_long res = sf2r_read_shift_register(iot, ioh);
	return (res>>25) & 0x03;
}

u_long
sf2r_read_shift_register(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	u_char i;
	u_long res = 0;
	u_char state = 0;

	bus_space_write_1(iot, ioh, 0, 0x05);
	DELAY(6);
	bus_space_write_1(iot, ioh, 0, 0x07);

	i = bus_space_read_1(iot, ioh, 0);
	DELAY(6);
	state = i & 0x80 ? 0x01 : 0; /* Amplifier: 0 - not present, 1 - present */
	state |= i & 0x08 ? 0 : 0x02; /* Signal: 0 - not tuned, 1 - tuned */

	bus_space_write_1(iot, ioh, 0, 0x05);
	i = bus_space_read_1(iot, ioh, 0);
	state |= i & 0x08 ? 0 : 0x01; /* Stereo: 0 - mono, 1 - stereo */
	res = i & 0x01;

	i = 23;
	while ( i-- ) {
		DELAY(6);
		res <<= 1;
		bus_space_write_1(iot, ioh, 0, 0x07);
		DELAY(6);
		bus_space_write_1(iot, ioh, 0, 0x05);
		res |= bus_space_read_1(iot, ioh, 0) & 0x01;
	}

	return res | (state<<25);
}

/*
 * Do hardware search
 */
void
sf2r_search(struct sf2r_softc *sc, u_char dir)
{
	u_long reg;
	u_int co = 0;

	reg = sc->sc_stereo | sc->sc_lock |
		TEA5757_SEARCH_START | dir ? TEA5757_SEARCH_UP : TEA5757_SEARCH_DOWN;
	sf2r_write_shift_register(sc, reg);

	DELAY(TEA5757_ACQUISITION_DELAY);
	do {
		DELAY(TEA5757_WAIT_DELAY);
		co++;
		reg = sf2r_read_shift_register(sc->sc_iot, sc->sc_ioh);
	} while ((reg & TEA5757_FREQ) == 0 && co < 200);

	if (co < 200)
		sc->sc_freq = tea5757_decode_freq(reg);
	else
		sf2r_set_freq(sc, sc->sc_freq);

	return;
}

/*
 * Convert frequency from hardware represenation
 */
static u_long
tea5757_decode_freq(u_long reg)
{
	reg &= TEA5757_FREQ;
	reg *= 125; /* 12.5 kHz */
	reg /= 10;
	reg -= IF_FREQ;
	return reg;
}
