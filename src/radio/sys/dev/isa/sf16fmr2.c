/* $RuOBSD: sf16fmr2.c,v 1.7 2001/09/30 16:45:07 pva Exp $ */

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

/*
 * Philips TEA5757H AM/FM Self Tuned Radio:
 *	http://www.semiconductors.philips.com/pip/TEA5757H
 */

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
#define SF16FMR2_CAPABILITIES	RADIO_CAPS_DETECT_STEREO |		\
				RADIO_CAPS_DETECT_SIGNAL |		\
				RADIO_CAPS_SET_MONO |			\
				RADIO_CAPS_LOCK_SENSITIVITY |		\
				RADIO_CAPS_HW_AFC |			\
				RADIO_CAPS_HW_SEARCH

#define SF16FMR2_AMPLIFIER	(1 << 7)
#define SF16FMR2_SIGNAL		(1 << 3)
#define SF16FMR2_STEREO		(1 << 3)

#define SF16FMR2_MUTE		0x00
#define SF16FMR2_UNMUTE		0x04

#define SF16FMR2_DATA_ON	(1 << 0)
#define SF16FMR2_DATA_OFF	(0 << 0)

#define SF16FMR2_CLCK_ON	(1 << 1)
#define SF16FMR2_CLCK_OFF	(0 << 1)

#define SF16FMR2_WREN_ON	(0 << 2)  /* SF16-FMR2 has inverse WREN */
#define SF16FMR2_WREN_OFF	(1 << 2)

#define SF16FMR2_WRITE_ZERO_CLOCK_LOW	\
		SF16FMR2_DATA_OFF | SF16FMR2_CLCK_OFF |	SF16FMR2_WREN_ON

#define SF16FMR2_WRITE_ZERO_CLOCK_HIGH	\
		SF16FMR2_DATA_OFF | SF16FMR2_CLCK_ON | SF16FMR2_WREN_ON

#define SF16FMR2_WRITE_ONE_CLOCK_LOW	\
		SF16FMR2_DATA_ON | SF16FMR2_CLCK_OFF | SF16FMR2_WREN_ON

#define SF16FMR2_WRITE_ONE_CLOCK_HIGH	\
		SF16FMR2_DATA_ON | SF16FMR2_CLCK_ON | SF16FMR2_WREN_ON

#define SF16FMR2_READ_CLCK_LOW		\
		SF16FMR2_DATA_ON | SF16FMR2_CLCK_OFF | SF16FMR2_WREN_OFF

#define SF16FMR2_READ_CLCK_HIGH		\
		SF16FMR2_DATA_ON | SF16FMR2_CLCK_ON | SF16FMR2_WREN_OFF

int	sf2r_probe(struct device *, void *, void *);
void	sf2r_attach(struct device *, struct device * self, void *);
int	sf2r_open(dev_t, int, int, struct proc *);
int	sf2r_close(dev_t, int, int, struct proc *);
int	sf2r_ioctl(dev_t, u_long, caddr_t, int, struct proc *);

/* define our interface to the higher level radio driver */
struct radio_hw_if sf2r_hw_if = {
	sf2r_open,
	sf2r_close,
	sf2r_ioctl
};

struct sf2r_softc {
	struct device	sc_dev;
	bus_space_tag_t sc_iot;
	bus_space_handle_t sc_ioh;

	u_long		sc_freq;
	u_char		sc_vol;
	u_char		sc_mute;
	u_long		sc_stereo;
	u_long		sc_lock;
};

struct cfattach sf2r_ca = {
	sizeof(struct sf2r_softc), sf2r_probe, sf2r_attach
};

struct cfdriver sf2r_cd = {
	NULL, "sf2r", DV_DULL
};

u_int	sf2r_find(bus_space_tag_t, bus_space_handle_t);

void	sf2r_set_mute(struct sf2r_softc *);
void	sf2r_set_freq(struct sf2r_softc *, u_long);
u_int	sf2r_state(bus_space_tag_t, bus_space_handle_t);
void	sf2r_search(struct sf2r_softc *, u_char);

void	sf2r_write_shift_register(struct sf2r_softc *, u_long);
u_long	sf2r_read_shift_register(bus_space_tag_t, bus_space_handle_t);

static u_long	tea5757_decode_freq(u_long);

int
sf2r_probe(struct device *parent, void *self, void *aux)
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
	sf2r_set_freq(sc, sc->sc_freq);

	radio_attach_mi(&sf2r_hw_if, sc, &sc->sc_dev);
}

int
sf2r_open(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct sf2r_softc *sc;
	return !(sc = sf2r_cd.cd_devs[0]) ? ENXIO : 0;
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
		*(u_long *)data = sc->sc_stereo == TEA5757_STEREO ? 0 : 1;
		break;
	case RIOCSMONO:
		sc->sc_stereo = *(u_long *)data ? TEA5757_MONO : TEA5757_STEREO;
		sf2r_set_freq(sc, sc->sc_freq);
		break;
	case RIOCGFREQ:
		sc->sc_freq = tea5757_decode_freq(
		    sf2r_read_shift_register(sc->sc_iot, sc->sc_ioh));
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
		/* FALLTHROUGH */
	case RIOCGREFF:
		/* NOT SUPPORTED */
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
			/* FALLTHROUGH */
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
 * Mute/unmute the card
 */
void
sf2r_set_mute(struct sf2r_softc *sc)
{
	u_char mute;

	mute = (sc->sc_mute || !sc->sc_vol) ? SF16FMR2_MUTE : SF16FMR2_UNMUTE;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, mute);
	DELAY(6);
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, mute);
}

void
sf2r_write_shift_register(struct sf2r_softc *sc, u_long data)
{
	int i = 25;

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, SF16FMR2_MUTE);

	while (i--)
		if (data & (1 << i)) {
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0,
				SF16FMR2_WRITE_ONE_CLOCK_LOW);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0,
				SF16FMR2_WRITE_ONE_CLOCK_HIGH);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0,
				SF16FMR2_WRITE_ONE_CLOCK_LOW);
		} else {
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0,
				SF16FMR2_WRITE_ZERO_CLOCK_LOW);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0,
				SF16FMR2_WRITE_ZERO_CLOCK_HIGH);
			bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0,
				SF16FMR2_WRITE_ZERO_CLOCK_LOW);
		}

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, SF16FMR2_MUTE);
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, SF16FMR2_UNMUTE);

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
		 * If the written and read frequencies are
		 * the same then success.
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
	return (res>>25) & 3;
}

u_long
sf2r_read_shift_register(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	u_char i;
	u_long res = 0;
	u_char state = 0;

	bus_space_write_1(iot, ioh, 0, SF16FMR2_READ_CLCK_LOW);
	DELAY(10);
	bus_space_write_1(iot, ioh, 0, SF16FMR2_READ_CLCK_HIGH);

	i = bus_space_read_1(iot, ioh, 0);
	DELAY(10);
	/* Amplifier: 0 - not present, 1 - present */
	state = i & SF16FMR2_AMPLIFIER ? (1 << 2) : (0 << 2);
	/* Signal: 0 - not tuned, 1 - tuned */
	state |= i & SF16FMR2_SIGNAL   ? (0 << 1) : (1 << 1);

	bus_space_write_1(iot, ioh, 0, SF16FMR2_READ_CLCK_LOW);
	i = bus_space_read_1(iot, ioh, 0);
	/* Stereo: 0 - mono, 1 - stereo */
	state |= i & SF16FMR2_STEREO   ? (0 << 0) : (1 << 0);
	res = i & SF16FMR2_DATA_ON;

	i = 23;
	while ( i-- ) {
		DELAY(10);
		res <<= 1;
		bus_space_write_1(iot, ioh, 0, SF16FMR2_READ_CLCK_HIGH);
		DELAY(10);
		bus_space_write_1(iot, ioh, 0, SF16FMR2_READ_CLCK_LOW);
		res |= bus_space_read_1(iot, ioh, 0) & SF16FMR2_DATA_ON;
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

	reg = sc->sc_stereo | sc->sc_lock | TEA5757_SEARCH_START;
	reg |= dir ? TEA5757_SEARCH_UP : TEA5757_SEARCH_DOWN;
	sf2r_write_shift_register(sc, reg);

	DELAY(TEA5757_ACQUISITION_DELAY);

	do {
		DELAY(TEA5757_WAIT_DELAY);
		reg = sf2r_read_shift_register(sc->sc_iot, sc->sc_ioh);
	} while ((reg & TEA5757_FREQ) == 0 && ++co < 400);
}

/*
 * Convert frequency from hardware represenation, should belong to tea5757.c
 */
static u_long
tea5757_decode_freq(u_long reg)
{
	reg &= TEA5757_FREQ;
	/* 12.5 kHz */
	reg *= 125;
	reg /= 10;
	reg -= IF_FREQ;
	return reg;
}
