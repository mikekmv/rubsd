/* $RuOBSD$ */

/*
 * Copyright (c) 2001 Maxim Tsyplakov <tm@oganer.net>
 * 		      Vladimir Popov <jumbo@narod.ru>
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

/* Guillemot Maxi Radio FM2000 PCI Radio Card Device Driver */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/radioio.h>

#include <machine/bus.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/ic/tea5757.h>
#include <dev/radio_if.h>

int	mr_match(struct device *, void *, void *);
void	mr_attach(struct device *, struct device *, void *);
int	mr_open(dev_t, int, int, struct proc *);
int	mr_close(dev_t, int, int, struct proc *);
int	mr_ioctl(dev_t, u_long, caddr_t, int, struct proc *);

/* config base I/O address ? */
#define PCI_CBIO 0x6400	

#define MAXIRADIO_CAPS	RADIO_CAPS_DETECT_SIGNAL |			\
			RADIO_CAPS_SET_MONO |				\
			RADIO_CAPS_HW_SEARCH |				\
			RADIO_CAPS_HW_AFC |				\
			RADIO_CAPS_LOCK_SENSITIVITY
#if 0
			RADIO_CAPS_DETECT_STEREO |
#endif /* 0 */

#define MAXIRADIO_MUTE		0x00
#define MAXIRADIO_UNMUTE	0x10

#define MAXIRADIO_SIGNAL	0x08

#define MR_WREN_ON		(1 << 2)
#define MR_WREN_OFF		(0 << 2)

#define MR_DATA_ON		(1 << 1)
#define MR_DATA_OFF		(0 << 1)

#define MR_CLCK_ON		(1 << 0)
#define MR_CLCK_OFF		(0 << 0)

#define MR_READ_CLOCK_LOW	(MR_WREN_OFF | MR_DATA_ON | MR_CLCK_OFF)
#define MR_READ_CLOCK_HIGH	(MR_WREN_OFF | MR_DATA_ON | MR_CLCK_ON)

/* define our interface to the high-level radio driver */

struct radio_hw_if mr_hw_if = {
	mr_open,
	mr_close,
	mr_ioctl
};

struct mr_softc {
	struct device	sc_dev;

	u_char	vol;
	u_char	mute;
	u_long	freq;
	u_long	stereo;
	u_long	lock;

	struct tea5757_t	tea;
};

struct cfattach mr_ca = {
	sizeof(struct mr_softc), mr_match, mr_attach
};

struct cfdriver mr_cd = {
	NULL, "mr", DV_DULL
};

void	mr_set_mute(struct mr_softc *);
void	mr_write_bit(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_char);
void	mr_init(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_long);
void	mr_rset(bus_space_tag_t, bus_space_handle_t, bus_size_t, u_long);
u_char	mr_state(bus_space_tag_t, bus_space_handle_t);
u_long	mr_hardware_read(bus_space_tag_t, bus_space_handle_t, bus_size_t);

int
mr_match(struct device *parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;
	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_GUILLEMOT &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_GUILLEMOT_MAXIRADIO)
		return (1);
	return (0);
}

void
mr_attach(struct device *parent, struct device *self, void *aux)
{
	struct mr_softc *sc = (struct mr_softc *) self;
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	bus_addr_t iobase;
	bus_size_t iosize;
	pcireg_t csr;
	
	if (pci_io_find(pc, pa->pa_tag, PCI_CBIO, &iobase, &iosize)) {
		printf (": can't find i/o base\n");
		return;
	}

	if (bus_space_map(sc->tea.iot = pa->pa_iot, iobase, iosize,
			  0, &sc->tea.ioh)) {
		printf(": can't map i/o space\n");
		return;
	}

	/* Enable the card */
	csr = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
	    csr | PCI_COMMAND_MASTER_ENABLE);
	
#ifdef	RADIO_INIT_FREQ
	sc->freq = RADIO_INIT_FREQ;
#else
	sc->freq = MIN_FM_FREQ;
#endif /* RADIO_INIT_FREQ */
#ifdef	RADIO_INIT_VOLU
	sc->vol = RADIO_INIT_VOLU;
#else
	sc->vol = 0;
#endif /* RADIO_INIT_VOLU */
#ifdef	RADIO_INIT_MUTE
	sc->mute = RADIO_INIT_MUTE;
#else
	sc->mute = 0;
#endif /* RADIO_INIT_MUTE */
	sc->stereo = TEA5757_STEREO;
	sc->lock = TEA5757_S030;
	sc->tea.offset = 0;
	sc->tea.init = mr_init;
	sc->tea.rset = mr_rset;
	sc->tea.write_bit = mr_write_bit;
	sc->tea.read = mr_hardware_read;
	
	radio_attach_mi(&mr_hw_if, sc, &sc->sc_dev);
}

int
mr_open(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct mr_softc *sc;
	return !(sc = mr_cd.cd_devs[0]) ? ENXIO : 0;
}

int
mr_close(dev_t dev, int flags, int fmt, struct proc *p)
{
	return 0;
}

/*
 * Handle the ioctl for the device
 */

int
mr_ioctl(dev_t dev, u_long cmd, caddr_t data, int flags, struct proc *p)
{
	struct mr_softc *sc = mr_cd.cd_devs[0];
	int error;

	error = 0;
	switch (cmd) {
	case RIOCGMUTE:
		*(u_long *)data = sc->mute ? 1 : 0;
		break;
	case RIOCSMUTE:
		sc->mute = *(u_long *)data ? 1 : 0;
		mr_set_mute(sc);
		break;
	case RIOCGVOLU:
		*(u_long *)data = sc->vol ? 255 : 0;
		break;
	case RIOCSVOLU:
		sc->vol = *(u_long *)data ? 1 : 0;
		mr_set_mute(sc);
		break;
	case RIOCGMONO:
		*(u_long *)data = sc->stereo == TEA5757_STEREO ? 0 : 1;
		break;
	case RIOCSMONO:
		sc->stereo = *(u_long *)data ? TEA5757_MONO : TEA5757_STEREO;
		tea5757_set_freq(&sc->tea, sc->lock, sc->stereo, sc->freq);
		mr_set_mute(sc);
		break;
	case RIOCGCAPS:
		*(u_long *)data = MAXIRADIO_CAPS;
		break;
	case RIOCGFREQ:
		sc->freq = tea5757_decode_freq(mr_hardware_read(sc->tea.iot,
					sc->tea.ioh, sc->tea.offset));
		*(u_long *)data = sc->freq;
		break;
	case RIOCSFREQ:
		sc->freq = tea5757_set_freq(&sc->tea, sc->lock, sc->stereo,
				*(u_long *)data);
		mr_set_mute(sc);
		break;
	case RIOCSSRCH:
		tea5757_search(&sc->tea, sc->lock, sc->stereo, *(u_long *)data);
		mr_set_mute(sc);
		break;
	case RIOCGLOCK:
		*(u_long *)data = tea5757_decode_lock(sc->lock);
		break;
	case RIOCSLOCK:
		sc->lock = tea5757_encode_lock(*(u_char *)data);
		tea5757_set_freq(&sc->tea, sc->lock, sc->stereo, sc->freq);
		mr_set_mute(sc);
		break;
	case RIOCGINFO:
		*(u_long *)data = mr_state(sc->tea.iot, sc->tea.ioh);
		break;
	case RIOCGREFF:
		/* FALLTHROUGH */
	case RIOCSREFF:
		/* FALLTHROUGH */
	default:
		error = EINVAL;	/* invalid agument */
	}
	return error;
}

void
mr_set_mute(struct mr_softc *sc)
{
	u_char mute;

	mute = (sc->mute || !sc->vol) ? MAXIRADIO_MUTE : MAXIRADIO_UNMUTE;
	bus_space_write_1(sc->tea.iot, sc->tea.ioh, 0, mute);
}

void
mr_write_bit(bus_space_tag_t iot, bus_space_handle_t ioh, bus_size_t off,
		u_char bit)
{
	u_char data;

	data = bit ? MR_DATA_ON : MR_DATA_OFF;
	bus_space_write_1(iot, ioh, off, MR_WREN_ON | MR_CLCK_OFF | data);
	DELAY(5);
	bus_space_write_1(iot, ioh, off, MR_WREN_ON | MR_CLCK_ON  | data);
	DELAY(5);
	bus_space_write_1(iot, ioh, off, MR_WREN_ON | MR_CLCK_OFF | data);
	DELAY(5);
}

void
mr_init(bus_space_tag_t iot, bus_space_handle_t ioh, bus_size_t off, u_long d)
{
	bus_space_write_1(iot, ioh, off, MR_WREN_ON | MR_DATA_ON | MR_CLCK_OFF);
}

void
mr_rset(bus_space_tag_t iot, bus_space_handle_t ioh, bus_size_t off, u_long d)
{
	bus_space_write_1(iot, ioh, off, MAXIRADIO_UNMUTE);
}

/* COMPLETELY UNTESTED */
u_long
mr_hardware_read(bus_space_tag_t iot, bus_space_handle_t ioh, bus_size_t off)
{
	u_long reg = 0ul;
	int i;

	bus_space_write_1(iot, ioh, off, MR_READ_CLOCK_LOW);
	DELAY(5);

	i = 24;
	while (i--) {
		bus_space_write_1(iot, ioh, off, MR_READ_CLOCK_HIGH);
		DELAY(5);
		bus_space_write_1(iot, ioh, off, MR_READ_CLOCK_LOW);
		DELAY(5);
		reg |= bus_space_read_1(iot, ioh, off) & MR_DATA_ON ? 1 : 0;
		reg <<= 1;
	}

	reg >>= 1;

	return (reg & (TEA5757_DATA | TEA5757_FREQ));
}

u_char
mr_state(bus_space_tag_t iot, bus_space_handle_t ioh)
{
	/* XXX: where is a stereo bit ? */
	return bus_space_read_1(iot, ioh, 0) & MAXIRADIO_SIGNAL ?
		(0 << 1) : (1 << 1);
}
