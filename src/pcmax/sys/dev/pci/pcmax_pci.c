/* $RuOBSD: pcmax_pci.c,v 1.5 2003/11/26 21:38:35 tm Exp $ */

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

#define	PCMAX_PCI_CBIO			(0x10)	/* I/O map BAR */
#define PCMAX_PCI_CMEM			(0x14)	/* memory map BAR */

int             pcmax_pci_match(struct device *, void *, void *);
void            pcmax_pci_attach(struct device *, struct device *, void *);

void	pcmax_set_scl_pci(struct pcmax_softc *);
void	pcmax_clr_scl_pci(struct pcmax_softc *);
void	pcmax_set_sda_pci(struct pcmax_softc *);
void	pcmax_clr_sda_pci(struct pcmax_softc *);

struct cfattach pcmax_ca = {
	sizeof(struct pcmax_pci_softc), pcmax_pci_match, pcmax_pci_attach
};

int
pcmax_pci_match(struct device * parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_TIGERJET &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_TIGER320)
		return (1);
	return (0);
}

void
pcmax_pci_attach(struct device * parent, struct device * self, void *aux)
{
	struct pcmax_softc *sc = (struct pcmax_softc *) self;
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	pcireg_t csr;
         
	if (pci_mapreg_map(pa, PCMAX_PCI_CMEM, PCI_MAPREG_TYPE_MEM,
			0, &sc->sc_pcmax.iot, &sc->sc_pcmax.ioh, NULL, NULL, 0)) {
		printf(": can't map memory space\n");
		return;
	}

	/* Enable the card */
	csr = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
		csr | PCI_COMMAND_MASTER_ENABLE);

	sc->sc_pcmax.set_scl = pcmax_set_scl_pci;
	sc->sc_pcmax.clr_scl = pcmax_clr_scl_pci;
	sc->sc_pcmax.set_sda = pcmax_set_sda_pci;
	sc->sc_pcmax.clr_sda = pcmax_clr_sda_pci;
	sc->sc_pcmax.mute = 0;
	sc->sc_pcmax.ioc = 0;

	/* enable I2C */
	sc->sc_pcmax.ioc |= PCMAX_PCI_I2C_MASK;
	bus_space_write_1(sc->sc_pcmax.iot, &sc->sc_pcmax.ioh, 
		PCMAX_PCI_CONTROL_OFFSET, sc->sc_pcmax.ioc);
	pcmax_set_mute(sc->sc_pcmax);
	printf(": Pcimax Ultra FM-Transmitter\n");
	radio_attach_mi(&pcmax_hw_if, sc, &sc->sc_pcmax.sc_dev);
}

void
pcmax_pci_set_mute(struct pcmax_softc * sc)
{
	if (!sc->mute)
		sc->sc_pcmax.ioc &= ~PCMAX_PCI_RF_MASK;
	else
		sc->sc_pcmax.ioc |= PCMAX_PCI_RF_MASK;
	bus_space_write_1(sc->sc_pcmax.iot, sc->sc_pcmax.ioh, 
		PCMAX_PCI_CONTROL_OFFSET, sc->sc_pcmax.ioc);
}

/* Set the SDA, pulling it high */
void
pcmax_set_sda_pci(struct pcmax_softc * sc)
{
	sc->iov |= PCMAX_PCI_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

/* Set the SCL, pulling it high*/
void 
pcmax_set_scl_pci(struct pcmax_softc * sc)
{
	sc->iov |= PCMAX_PCI_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

void 
clr_sda_pci(struct pcmax_softc * sc)
{
	sc->iov &= ~PCMAX_PCI_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

/* Clear the SCL bit */
void
clr_scl_pci(struct pcmax_softc * sc)
{
	sc->iov &= ~PCMAX_PCI_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}
