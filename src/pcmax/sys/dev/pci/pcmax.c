/* $RuOBSD$ */

/*
 * Copyright (c) 2003 Maxim Tsyplakov <tm@oganer.net>
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

/* Pcimax Ultra FM-transmitter driver */

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

#include <dev/radio_if.h>

int             pcmax_match(struct device *, void *, void *);
void            pcmax_attach(struct device *, struct device *, void *);

int             pcmax_get_info(void *, struct radio_info *);
int             pcmax_set_info(void *, struct radio_info *);

#define PCMAX_CAPS	RADIO_CAPS_DETECT_SIGNAL |			\
			RADIO_CAPS_DETECT_STEREO |			\
			RADIO_CAPS_SET_MONO |				\
			RADIO_CAPS_HW_SEARCH |				\
			RADIO_CAPS_HW_AFC |				\
			RADIO_CAPS_LOCK_SENSITIVITY

struct radio_hw_if pcmax_hw_if = {
	NULL,			/* open */
	NULL,			/* close */
	pcmax_get_info,
	pcmax_set_info,
	NULL,			/* search */
};

struct pcmax_softc {
	struct device   sc_dev;

	int             mute;
	u_int8_t        vol;
	u_int32_t       freq;
	u_int32_t       stereo;
	u_int32_t       lock;
};

struct cfattach pcmax_ca = {
	sizeof(struct pcmax_softc), pcmax_match, pcmax_attach
};

struct cfdriver pcmax_cd = {
	NULL, "pcmax", DV_DULL
};

int
pcmax_match(struct device * parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_TIGERJET &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_TIGERJECT_PCIIF)
		return (1);
	return (0);
}

void
pcmax_attach(struct device * parent, struct device * self, void *aux)
{
	struct pcmax_softc *sc = (struct pcmax_softc *) self;

	/* write an initial zero to the port */
	pcmax_write_power(0);
	printf(": Pcimax Ultra FM-Transmitter\n");

	radio_attach_mi(&pcmax_hw_if, sc, &sc->sc_dev);
}

int
pcmax_get_info(void *v, struct radio_info * ri)
{
	return (0);
}

int
gtp_set_info(void *v, struct radio_info * ri)
{
	return (0);
}

static void
pcmax_write_power(int power)
{
	if (pcmax_pci) {
		/* PCMAX PCI only has two modes: Power on, power off */
		if (power == 0) {
			io_control &= (~PCMAX_PCI_RF_MASK);
#ifdef PCMAX_USE_PCI1	/* i plan realize it via flags */
			writeb(io_control, PCMAX_PCI_CONTROL_PORT(io_port));
#else
			outb(io_control, PCMAX_PCI_CONTROL_PORT(io_port));
#endif
		} else {
			io_control |= (PCMAX_PCI_RF_MASK);
#ifdef PCMAX_USE_PCI1
			writeb(io_control, PCMAX_PCI_CONTROL_PORT(io_port));
#else
			outb(io_control, PCMAX_PCI_CONTROL_PORT(io_port));
#endif
		}

	}
}
