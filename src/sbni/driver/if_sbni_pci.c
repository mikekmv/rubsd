/*	$RuOBSD$	*/

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

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include <sys/device.h>
#include <sys/timeout.h>

#include <net/if.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/if_ether.h>
#endif

#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>

#include <dev/ic/sbnireg.h>
#include <dev/ic/sbnivar.h>

/*
 * PCI constants.
 * XXX These should be in a common file!
 */
#define PCI_CBIO		0x10	/* Configuration Base IO Address */

#define PCI_VENDOR_GRANCH	0x55
#define PCI_PRODUCT_SBNI	0x9f

struct sbni_pci_softc {
	struct sbni_softc sc_sbni;

	void *sc_ih;
};

static int sbni_pci_match __P((struct device *, void *, void *));
static void sbni_pci_attach __P((struct device *, struct device *, void *));

struct cfattach sbni_pci_ca = {
	sizeof(struct sbni_pci_softc), sbni_pci_match, sbni_pci_attach
};

static int
sbni_pci_match(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) == PCI_VENDOR_GRANCH &&
	    PCI_PRODUCT(pa->pa_id) == PCI_PRODUCT_SBNI)
		return (1);

	return (0);
}

static void
sbni_pci_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct sbni_pci_softc *psc = (void *)self;
	struct sbni_softc *sc = &psc->sc_sbni;
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	bus_addr_t iobase;
	bus_size_t iosize;
	pcireg_t csr;
	pci_intr_handle_t ih;
	const char *intrstr;

	if (pci_io_find(pc, pa->pa_tag, PCI_CBIO, &iobase, &iosize)) {
		printf(": can't find i/o base\n");
		return;
	}

	if (bus_space_map(sc->sc_iot = pa->pa_iot, iobase, iosize, 0,
	    &sc->sc_ioh)) {
		printf(": can't map i/o space\n");
		return;
	}

	/* Enable the card */
	csr = pci_conf_read(pc, pa->pa_tag,
	    PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
	    csr | PCI_COMMAND_MASTER_ENABLE);

	/* Map and establish the interrupt */
	if (pci_intr_map(pc, pa->pa_intrtag, pa->pa_intrpin,
	    pa->pa_intrline, &ih)) {
		printf(": couldn't map interrupt\n");
		return;
	}
	intrstr = pci_intr_string(pc, ih);
	psc->sc_ih = pci_intr_establish(pc, ih, IPL_NET, sbni_intr, sc,
		sc->sc_dev.dv_xname);
	if (psc->sc_ih == NULL) {
		printf(": couldn't establish interrupt");
		if (intrstr != NULL)
			printf(" at %s", intrstr);
		printf("\n");
		return;
	}
	printf(": %s\n", intrstr);

	/* Do bus-independent attach */
	sbni_attach(sc);
}
