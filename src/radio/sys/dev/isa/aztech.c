/* $RuOBSD: aztech.c,v 1.2 2001/09/29 03:47:04 pva Exp $ */

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

/* Aztech/PackardBell FM Radio Card device driver */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/radioio.h>

#include <machine/bus.h>

#include <dev/isa/isavar.h>
#include <dev/radio_if.h>

#define RF_25K	25
#define RF_50K	50
#define RF_100K	100

#define MAX_VOL	3
#define VOLUME_RATIO(x)	(255*x/MAX_VOL)

#define AZ_BASE_VALID(x)	((x == 0x350) || (x == 0x358))
#define AZTECH_CAPABILITIES	\
					RADIO_CAPS_DETECT_STEREO | \
					RADIO_CAPS_DETECT_SIGNAL | \
					RADIO_CAPS_SET_MONO | \
					RADIO_CAPS_REFERENCE_FREQ

int     az_probe(struct device *, void *, void *);
void    az_attach(struct device *, struct device * self, void *);
int     az_open(dev_t, int, int, struct proc *);
int     az_close(dev_t, int, int, struct proc *);
int     az_ioctl(dev_t, u_long, caddr_t, int, struct proc *);

struct radio_hw_if az_hw_if = {
	az_open,
	az_close,
	az_ioctl
};

struct az_softc {
	struct device      sc_dev;
	bus_space_tag_t    sc_iot;
	bus_space_handle_t sc_ioh;

	u_long             sc_freq;
	u_char             sc_rf;
	u_char             sc_vol;
	u_char             sc_muted;
	u_char             sc_stereo;
};

struct cfattach az_ca = {
	sizeof(struct az_softc), az_probe, az_attach
};

struct cfdriver az_cd = {
	NULL, "az", DV_DULL
};

u_int   az_find(bus_space_tag_t, bus_space_handle_t);
void    az_reset(struct az_softc *);
void    az_set_mute(struct az_softc *);
void    az_set_freq(struct az_softc *, u_long);
u_char  az_state(bus_space_tag_t, bus_space_handle_t);

void    az_send_one(bus_space_tag_t, bus_space_handle_t, u_char);
void    az_send_zero(bus_space_tag_t, bus_space_handle_t, u_char);

u_char  az_conv_vol(u_char);
u_char  az_unconv_vol(u_char);

int
az_probe(struct device *parent, void *self, void *aux)
{
	struct isa_attach_args *ia = aux;
	bus_space_tag_t iot = ia->ia_iot;
	bus_space_handle_t ioh;

	int iosize = 1, iobase = ia->ia_iobase;

	if (!AZ_BASE_VALID(iobase)) {
		printf("az: configured iobase 0x%x invalid", iobase);
		return 0;
	}

	if (bus_space_map(iot, iobase, iosize, 0, &ioh))
			return 0;

	bus_space_unmap(iot, ioh, iosize);

	if (!az_find(iot, ioh))
		return 0;

	ia->ia_iosize = iosize;
	return 1;
}

void
az_attach(struct device *parent, struct device *self, void *aux)
{
	struct az_softc *sc = (void *) self;
	struct isa_attach_args *ia = aux;

	sc->sc_iot = ia->ia_iot;
	sc->sc_rf = RF_50K;
#ifdef RADIO_INIT_MUTE
	sc->sc_muted = RADIO_INIT_MUTE;
#else
	sc->sc_muted = 0;
#endif /* RADIO_INIT_MUTE */
#ifdef RADIO_INIT_FREQ
	sc->sc_freq = RADIO_INIT_FREQ;
#else
	sc->sc_freq = MIN_FM_FREQ;
#endif /* RADIO_INIT_FREQ */
	sc->sc_stereo = 1;
#ifdef RADIO_INIT_VOL
	sc->sc_vol = az_conv_vol(RADIO_INIT_VOL);
#else
	sc->sc_vol = 0;
#endif /* RADIO_INIT_VOL */

	/* remap I/O */
	if (bus_space_map(sc->sc_iot, ia->ia_iobase, ia->ia_iosize,
			  0, &sc->sc_ioh))
		panic(": bus_space_map() of %s failed", sc->sc_dev.dv_xname);

	printf(": Aztech/PackardBell");
	az_reset(sc);

	radio_attach_mi(&az_hw_if, sc, &sc->sc_dev);
}

int
az_open(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct az_softc *sc;
	return !(sc = az_cd.cd_devs[0]) ? ENXIO : 0;
}

int
az_close(dev_t dev, int flags, int fmt, struct proc *p)
{
	return 0;
}

/*
 * Handle the ioctl for the device
 */
int
az_ioctl(dev_t dev, u_long cmd, caddr_t data, int flags, struct proc *p)
{
	int error;
	struct az_softc *sc = az_cd.cd_devs[0];

	error = 0;
	switch (cmd) {
	case RIOCGMUTE:
		*(u_long *)data = sc->sc_muted ? 1 : 0;
		break;
	case RIOCSMUTE:
		sc->sc_muted = *(u_long *)data ? 1 : 0;
		az_set_mute(sc);
		break;
	case RIOCGVOLU:
		*(u_long *)data = az_unconv_vol(sc->sc_vol);
		break;
	case RIOCSVOLU:
		sc->sc_vol = az_conv_vol(*(u_int *)data);
		az_set_mute(sc);
		break;
	case RIOCGMONO:
		*(u_long *)data = sc->sc_stereo ? 0 : 1;
		break;
	case RIOCSMONO:
		sc->sc_stereo = *(u_long *)data ? 0 : 1;
		az_set_freq(sc, sc->sc_freq);
		break;
	case RIOCGFREQ:
		*(u_long *)data = sc->sc_freq;
		break;
	case RIOCSFREQ:
		az_set_freq(sc, *(u_long *)data);
		break;
	case RIOCSSRCH:
		/* NOT SUPPORTED */
		error = ENODEV;
		break;
	case RIOCGCAPS:
		*(u_long *)data = AZTECH_CAPABILITIES;
		break;
	case RIOCGINFO: /* Get Info */
		*(u_long *)data = 0x03 &
			(3 ^ az_state(sc->sc_iot, sc->sc_ioh));
		break;
	case RIOCSREFF:
		if (*(u_long *)data < 1 + (RF_50K + RF_25K) / 2)
			sc->sc_rf = RF_25K;
		else
			if (*(u_long *)data > (RF_50K + RF_25K) / 2 &&
				*(u_long *)data < (RF_100K + RF_50K) / 2)
				sc->sc_rf = RF_50K;
		else
			sc->sc_rf = RF_100K;
		az_set_freq(sc, sc->sc_freq);
		break;
	case RIOCGREFF:
		*(u_long *)data = sc->sc_rf;
		break;
	case RIOCSLOCK:
	case RIOCGLOCK:
		/* NOT SUPPORTED */
		error = ENODEV;
		break;
	default:
		error = EINVAL;
	}
	return (error);
}

/*
 * Mute the card
 */
void
az_set_mute(struct az_softc *sc)
{
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->sc_muted ? 0 : sc->sc_vol);
	DELAY(6);
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->sc_muted ? 0 : sc->sc_vol);
}

void
az_send_zero(bus_space_tag_t iot, bus_space_handle_t ioh, u_char vol)
{
	bus_space_write_1(iot, ioh, 0, 0x02 + vol);
	bus_space_write_1(iot, ioh, 0, 0x40 + 0x02 + vol);
}

void
az_send_one(bus_space_tag_t iot, bus_space_handle_t ioh, u_char vol)
{
	bus_space_write_1(iot, ioh, 0, 0x80 + 0x02 + vol);
	bus_space_write_1(iot, ioh, 0, 0x80 + 0x40 + 0x02 + vol);
}

void
az_set_freq(struct az_softc *sc, u_long nfreq)
{
	int i;
	u_char vol = sc->sc_muted ? 0 : sc->sc_vol;

	if (nfreq > MAX_FM_FREQ)
		nfreq = MAX_FM_FREQ;
	if (nfreq < MIN_FM_FREQ)
		nfreq = MIN_FM_FREQ;

	sc->sc_freq = nfreq;

	nfreq += IF_FREQ;
	nfreq /= sc->sc_rf;

	/* 14 frequency bits */
	for (i = 0; i < 14; i++)
		if (nfreq & (1<<i))
			az_send_one(sc->sc_iot, sc->sc_ioh, vol);
		else
			az_send_zero(sc->sc_iot, sc->sc_ioh, vol);

	/* LSI test bits */
	az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
	az_send_zero(sc->sc_iot, sc->sc_ioh, vol);

	/* Band */
	az_send_one(sc->sc_iot, sc->sc_ioh, vol);
	if (sc->sc_stereo)
		az_send_one(sc->sc_iot, sc->sc_ioh, vol);
	else
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
	az_send_zero(sc->sc_iot, sc->sc_ioh, vol);

	/* Time base */
	az_send_zero(sc->sc_iot, sc->sc_ioh, vol);

	/* Reference frequency */
	switch (sc->sc_rf) {
	case RF_25K:
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		az_send_one(sc->sc_iot, sc->sc_ioh, vol);
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		break;
	case RF_100K:
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		break;
	case RF_50K:
	default:
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		az_send_zero(sc->sc_iot, sc->sc_ioh, vol);
		az_send_one(sc->sc_iot, sc->sc_ioh, vol);
		break;
	}

	/* Do FM */
	az_send_one(sc->sc_iot, sc->sc_ioh, vol);

	bus_space_write_1(sc->sc_iot , sc->sc_ioh, 0, 0x80 + 0x40 + vol);
}

/*
 * Return state of the card - tuned/not tuned, mono/stereo
 */
u_char
az_state(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	return bus_space_read_1(iot, ioh, 0) & 3;
}

/*
 * Convert volume to hardware representation
 * The card uses bits 00000x0x to set volume
 */
u_char
az_conv_vol(u_char vol)
{
	if (vol < VOLUME_RATIO(1))
		return 0;
	else if (vol >= VOLUME_RATIO(1) && vol < VOLUME_RATIO(2))
		return 1;
	else if (vol >= VOLUME_RATIO(2) && vol < VOLUME_RATIO(3))
		return 4;
	else
		return 5;
}

/*
 * Convert volume from hardware representation
 */
u_char
az_unconv_vol(u_char vol)
{
	switch (vol) {
	case 0:
		return VOLUME_RATIO(0);
	case 1:
		return VOLUME_RATIO(1);
	case 4:
		return VOLUME_RATIO(2);
	}
	return VOLUME_RATIO(3);
}

void
az_reset(struct az_softc *sc)
{
	az_set_freq(sc, sc->sc_freq);
	az_set_mute(sc);
}

u_int
az_find(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	struct az_softc sc;
	u_int i, scanres = 0;

	sc.sc_iot = iot;
	sc.sc_ioh = ioh;
	sc.sc_rf = RF_50K;
	sc.sc_muted = 0;
	sc.sc_stereo = 1;
	sc.sc_vol = 0;

	/*
	 * Scan whole FM range. If there is a card it'll
	 * respond on some frequency
	 */
	for (i = MIN_FM_FREQ; !scanres && i < MAX_FM_FREQ; i += 10) {
		az_set_freq(&sc, i);
		scanres += 3 - az_state(iot, ioh);
	}

	return scanres;
}
