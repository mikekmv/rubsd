/* $RuOBSD: pcmax_pci.c,v 1.9 2003/11/27 14:59:33 tm Exp $ */

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

#include <dev/ic/pcmaxreg.h>
#include <dev/ic/pcmaxvar.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/radio_if.h>

#define	PCMAX_PCI_CBIO			(0x10)	/* I/O map BAR */
#define PCMAX_PCI_CMEM			(0x14)	/* memory map BAR */

int             pcmax_pci_match(struct device *, void *, void *);
void            pcmax_pci_attach(struct device *, struct device *, void *);

void		pcmax_pci_set_scl(struct pcmax_softc *);
void		pcmax_pci_clr_scl(struct pcmax_softc *);
void		pcmax_pci_set_sda(struct pcmax_softc *);
void		pcmax_pci_clr_sda(struct pcmax_softc *);
void		pcmax_pci_write_power(struct pcmax_softc *, u_int32_t);
u_int8_t	pcmax_pci_read_power(struct pcmax_softc *);

struct cfattach pcmax_pci_ca = {
	sizeof(struct pcmax_softc), pcmax_pci_match, pcmax_pci_attach
};

int
pcmax_pci_match(struct device * parent, void *match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_TIGERJET &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_TIGERJET_TIGER320)
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
			0, &sc->sc_iot, &sc->sc_ioh, NULL, NULL, 0)) {
		printf(": can't map memory space\n");
		return;
	}

	/* Enable the card */
	csr = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
		csr | PCI_COMMAND_MASTER_ENABLE);

	sc->set_scl = pcmax_pci_set_scl;
	sc->clr_scl = pcmax_pci_clr_scl;
	sc->set_sda = pcmax_pci_set_sda;
	sc->clr_sda = pcmax_pci_clr_sda;
	sc->write_power = pcmax_pci_write_power;	
	sc->read_power = pcmax_pci_read_power;
	
	/* enable I2C */
	sc->ioc |= PCMAX_PCI_I2C_MASK;
	/* bus_space_write_1(&sc->sc_iot, &sc->sc_ioh, PCMAX_PCI_CONTROL_OFFSET,
		sc->ioc); */
	printf(": Pcimax Ultra FM-Transmitter\n");
	pcmax_attach(sc);
}

/* Set the SDA, pulling it high */
void
pcmax_pci_set_sda(struct pcmax_softc * sc)
{
	sc->iov |= PCMAX_PCI_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

/* Set the SCL, pulling it high*/
void 
pcmax_pci_set_scl(struct pcmax_softc * sc)
{
	sc->iov |= PCMAX_PCI_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

void 
pcmax_pci_clr_sda(struct pcmax_softc * sc)
{
	sc->iov &= ~PCMAX_PCI_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

/* Clear the SCL bit */
void
pcmax_pci_clr_scl(struct pcmax_softc * sc)
{
	sc->iov &= ~PCMAX_PCI_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET,
		sc->iov);
}

/* PCI device has only two power modes (on/off) */
void
pcmax_pci_write_power(struct pcmax_softc * sc, u_int32_t p)
{
	if (!p)
		sc->ioc &= ~PCMAX_PCI_RF_MASK;
	else
		sc->ioc |= PCMAX_PCI_RF_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 
		PCMAX_PCI_CONTROL_OFFSET, sc->ioc);
}

u_int8_t
pcmax_pci_read_power(struct pcmax_softc * sc)
{
	return (sc->vol);	/* stub */
}
