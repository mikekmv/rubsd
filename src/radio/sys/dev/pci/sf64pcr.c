/*
 * Copyright (c) 2001 Vladimir Popov <jumbo@narod.ru>
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
 */
/* MediaForte SoundForte SF64-PCR PCI Radio Card Device Driver */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/radioio.h>

#include <machine/bus.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/radio_if.h>
#include <dev/tea5757.h>

/* config base I/O address ? */
#define PCI_CBIO 0x6400	

#define CARD_RADIO_CAPS	RADIO_CAPS_DETECT_STEREO | \
			RADIO_CAPS_DETECT_SIGNAL | \
			RADIO_CAPS_SET_MONO | \
			RADIO_CAPS_HW_SEARCH | \
			RADIO_CAPS_HW_AFC | \
			RADIO_CAPS_LOCK_SENSITIVITY

int     sf64pcr_match __P((struct device *, void *, void *));
void    sf64pcr_attach __P((struct device *, struct device * self, void *));
int     sf64pcr_open __P((dev_t, int, int, struct proc *));
int     sf64pcr_close __P((dev_t, int, int, struct proc *));
int     sf64pcr_ioctl __P((dev_t, u_long, caddr_t, int, struct proc *));

/* define our interface to the high-level radio driver */
struct radio_hw_if mr_hw_if = {
	sf64pcr_open,
	sf64pcr_close,
	sf64pcr_ioctl
};

struct sf64pcr_softc {
	bus_space_tag_t    iot;
	bus_space_handle_t ioh;
	bus_size_t         offset;
	struct device      dev;

	u_char             mute;
	u_char             vol;
	u_long             freq;
	u_long             stereo;
	u_long             lock;
};

struct cfattach sf4r_ca = {
	sizeof(struct sf64pcr_softc), sf64pcr_match, sf64pcr_attach
};

struct cfdriver sf4r_cd = {
	NULL, "sf4r", DV_DULL
};

/*
 * Function prototypes
 */
void      sf64pcr_search __P((struct sf64pcr_softc *, u_char));
void      sf64pcr_set_mute __P((struct sf64pcr_softc *));
void      sf64pcr_set_freq __P((struct sf64pcr_softc *, u_long));
void      sf64pcr_hw_write __P((struct sf64pcr_softc *, u_long));
u_int32_t sf64pcr_hw_read __P((bus_space_tag_t, bus_space_handle_t, bus_size_t));

/*
 * PCI initialization stuff
 */
int
sf64pcr_match(parent, match, aux)
	struct device   *parent;
	void            *match, *aux;
{
	struct pci_attach_args *pa = aux;
	/* FIXME: add more thorough testing */
	/* desc = "SoundForte RadioLink SF64-PCR PCI"; */
	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_FORTEMEDIA &&
		PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_FORTEMEDIA_FM801)
		return (1);
	return (0);
}

void
sf64pcr_attach(parent, self, aux)
	struct device   *parent, *self;
	void            *aux;
{
	struct sf64pcr_softc *sc = (struct sf64pcr_softc *) self;
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	bus_addr_t iobase;
	bus_size_t iosize;
	pcireg_t csr;

	if (pci_io_find(pc, pa->pa_tag, PCI_CBIO, &iobase, &iosize)) {
		printf (": can't find i/o base\n");
		return;
	}

	if (bus_space_map(sc->iot = pa->pa_iot, iobase, iosize, 0, &sc->ioh)) {
		printf(": can't map i/o space\n");
		return;
	}

	/* Enable the card */
	csr = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
			csr | PCI_COMMAND_MASTER_ENABLE);

#ifdef RADIO_INIT_MUTE
	sc->mute = RADIO_INIT_MUTE;
#else
	sc->mute = 0;
#endif /* RADIO_INIT_MUTE */
#ifdef RADIO_INIT_FREQ
	sc->freq = RADIO_INIT_FREQ;
#else
	sc->freq = MIN_FM_FREQ;
#endif /* RADIO_INIT_FREQ */
	sc->stereo = TEA5757_STEREO;
	sc->lock = TEA5757_S030;
	sf64pcr_set_freq(sc, sc->freq);

	radio_attach_mi(&mr_hw_if, sc, &sc->dev);
}

int
sf64pcr_open(dev, flags, fmt, p)
	dev_t        dev;
	int          flags, fmt;
	struct proc *p;
{
	struct sf64pcr_softc *sc;
	return  !(sc = sf4r_cd.cd_devs[0]) ? ENXIO : 0;
}

int
sf64pcr_close(dev, flags, fmt, p)
	dev_t        dev;
	int          flags, fmt;
	struct proc	*p;
{
	return 0;
}

void
sf64pcr_search(sc, dir)
	struct sf64pcr_softc *sc;
	u_char                dir;
{
	u_long reg = 0;
	u_char co = 0;

	reg = sc->stereo | sc->lock | TEA5757_SEARCH_START |
		 dir ? TEA5757_SEARCH_UP : TEA5757_SEARCH_DOWN;
	sf64pcr_hw_write(sc, reg);

	DELAY(TEA5757_ACQUISITION_DELAY);
	do {
		DELAY(TEA5757_WAIT_DELAY);
		reg = sf64pcr_hw_read(sc->iot, sc->ioh, sc->offset);
	} while ((reg & TEA5757_FREQ) == 0 && co++ < 200);

	if (co < 200) {
		reg &= TEA5757_FREQ;
		reg *= 125;
		reg /= 10;
		reg -= 10700;
		sc->freq = reg;
	}

	sf64pcr_set_freq(sc, sc->freq);
	sf64pcr_set_mute(sc);

	return;
}

void
sf64pcr_set_mute(sc)
	struct sf64pcr_softc *sc;
{
	u_long mute = sc->mute ? 0xf800 : 0xf802;
	bus_space_write_2(sc->iot, sc->ioh, sc->offset, mute);
}

void
sf64pcr_set_freq(sc, freq)
	struct sf64pcr_softc *sc;
	u_long                freq;
{
	u_long reg = 0;

	if (freq < MIN_FM_FREQ)
		freq = MIN_FM_FREQ;
	if (freq > MAX_FM_FREQ)
		freq = MAX_FM_FREQ;
	
	sc->freq = freq;

	freq += 10700;
	freq *= 10;
	freq /= 125;

	reg = TEA5757_SEARCH_END | sc->lock | sc->stereo | freq;
	sf64pcr_hw_write(sc, reg);
}

void
sf64pcr_hw_write(sc, data)
	struct sf64pcr_softc *sc;
	u_long                data;
{
	int i = 25;
	
	bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf800);
	DELAY(4);

	while (i--)
		if (data & (1<<i)) {
			bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf804);
			DELAY(4);
			bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf805);
			DELAY(4);
			bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf804);
			DELAY(4);
		} else {
			bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf800);
			DELAY(4);
			bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf801);
			DELAY(4);
			bus_space_write_2(sc->iot, sc->ioh, sc->offset, 0xf800);
			DELAY(4);
		}

	sf64pcr_set_mute(sc);
}

u_int32_t
sf64pcr_hw_read(iot, ioh, offset)
	bus_space_tag_t    iot;
	bus_space_handle_t ioh;
	bus_size_t         offset;
{
	u_int32_t res = 0ul;
	int rb, ind = 0;

	bus_space_write_2(iot, ioh, offset, 0xfc02);
	DELAY(4); 

	/* Read the register */
	rb = 23;
	while (rb--) {
		bus_space_write_2(iot, ioh, offset, 0xfc03);
		DELAY(4);			

		bus_space_write_2(iot, ioh, offset, 0xfc02);
		DELAY(4);

		res |= bus_space_read_2(iot, ioh, offset) & 0x04 ? 1 : 0;
		res <<= 1;
	}

	bus_space_write_2(iot, ioh, offset, 0xfc03);
	DELAY(4);			

	rb = bus_space_read_1(iot, ioh, offset);
	ind = rb & 0x08 ? 1 : 0; /* Tuning */
	ind <<= 1;

	bus_space_write_2(iot, ioh, offset, 0xfc02);

	rb = bus_space_read_2(iot, ioh, offset);
	ind |= rb & 0x08 ? 1 : 0; /* Mono */
	res |= rb & 0x04 ? 1 : 0;

	return (res&(TEA5757_DATA|TEA5757_FREQ)) | (ind<<24);
}

int
sf64pcr_ioctl(dev, cmd, arg, flag, pr)
	dev_t          dev;
	u_long         cmd;
	caddr_t        arg;
	int            flag;
	struct proc   *pr;
{
	struct sf64pcr_softc *sc = sf4r_cd.cd_devs[0];
	int error = 0;
	u_long freq;

	switch (cmd) {
	case RIOCGINFO:
		freq = sf64pcr_hw_read(sc->iot, sc->ioh, sc->offset);
		*(int *)arg = 0;
		if (freq & (1<<24))
			*(int *)arg |= !RADIO_INFO_STEREO;
		if (freq & (1<<25))
			*(int *)arg |= !RADIO_INFO_SIGNAL;
		sf64pcr_set_freq(sc, sc->freq);
		break;
	case RIOCGMONO:
		if (sc->stereo == TEA5757_STEREO)
			*(int *)arg = 0;
		else
			*(int *)arg = 1;
		break;
	case RIOCSMONO:
		if (*(int *)arg)
			sc->stereo = TEA5757_MONO;
		else
			sc->stereo = TEA5757_STEREO;
		sf64pcr_set_freq(sc, sc->freq);
		break;
	case RIOCGCAPS:
		*(int *)arg = CARD_RADIO_CAPS;
		break;
	case RIOCGVOLU:
		*(int *)arg = sc->mute ? 0 : 255;
		break;
	case RIOCGMUTE:
		*(int *)arg = sc->mute ? 255 : 0;
		break;
	case RIOCSVOLU:
		sc->mute = *(int *)arg ? 0 : 1;
		sf64pcr_set_mute(sc);
		break;
	case RIOCSMUTE:
		sc->mute = *(int *)arg ? 1 : 0;
		sf64pcr_set_mute(sc);
		break;
	case RIOCGFREQ:
		freq = TEA5757_FREQ & sf64pcr_hw_read(sc->iot, sc->ioh, sc->offset);
		/* NO FLOATING POINT PLEASE */
		freq *= 125;
		freq /= 10;
		freq -= 10700;
		*(int *)arg = freq;
		sc->freq = freq;
		break;
	case RIOCSFREQ:
		sf64pcr_set_freq(sc, *(int *)arg);
		break;
	case RIOCGLOCK:
		switch (sc->lock) {
		case TEA5757_S005:
			*(int *)arg = 5;
			break;
		case TEA5757_S010:
			*(int *)arg = 10;
			break;
		case TEA5757_S030:
			*(int *)arg = 30;
			break;
		case TEA5757_S150:
			*(int *)arg = 150;
			break;
		}
		break;
	case RIOCSLOCK:
		if (*(int *)arg < 8)
			sc->lock = TEA5757_S005;
		else if (*(int *)arg > 7 && *(int *)arg < 21)
			sc->lock = TEA5757_S010;
		else if (*(int *)arg > 20 && *(int *)arg < 51)
			sc->lock = TEA5757_S030;
		else if (*(int *)arg > 50)
			sc->lock = TEA5757_S150;
		sf64pcr_set_freq(sc, sc->freq);
		break;
	case RIOCSSRCH:
		sf64pcr_search(sc, *(int *)arg);
		break;
	case RIOCSREFF: /* not supported */
	case RIOCGREFF:
	default:
		error = ENODEV;
	}
	return (error);
}
