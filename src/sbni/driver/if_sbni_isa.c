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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/syslog.h>
#include <sys/select.h>
#include <sys/device.h>
#include <sys/timeout.h>

#include <net/if.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#ifdef NS
#include <netns/ns.h>
#include <netns/ns_if.h>
#endif

#if NBPFILTER > 0
#include <net/bpf.h>
#include <net/bpfdesc.h>
#endif

#include <machine/intr.h>
#include <machine/bus.h>

#include <dev/ic/sbnireg.h>
#include <dev/ic/sbnivar.h>

#include <dev/isa/isavar.h>

static int	sbni_isa_match __P((struct device *, void *, void *));
static void	sbni_isa_attach __P((struct device *, struct device *, void *));
static int	sbni_detect __P((struct sbni_softc *sc));

struct sbni_isa_softc {
	struct	sbni_softc sc_sbni;
	void	*sc_ih;
};

struct cfattach sbni_isa_ca = {
	sizeof(struct sbni_isa_softc), sbni_isa_match, sbni_isa_attach
};

static int
sbni_isa_match(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	struct sbni_isa_softc *isc = match;
	struct sbni_softc *sc = &isc->sc_sbni;
	struct isa_attach_args *ia = aux;
	int rv;

	/* Disallow wildcarded values */
	if (ia->ia_irq ==  -1 || ia->ia_iobase == -1)
		return (0);

	/* Make sure this is a valid SBNI 12-xx i/o address */
	if ((ia->ia_iobase & 3) != 0 || ia->ia_iobase < 0x210 ||
	    ia->ia_iobase > 0x2f0)
		return (0);

	/* Map i/o space */
	if (bus_space_map(sc->sc_iot, ia->ia_iobase, SBNI_NPORTS, 0,
	    &sc->sc_ioh))
		return (0);

	if ((rv = sbni_detect(sc)) != 0)
		ia->ia_iosize = SBNI_NPORTS;

	/* Unmap i/o space */
	bus_space_unmap(sc->sc_iot, sc->sc_ioh, SBNI_NPORTS);

	return (rv);
}

static void
sbni_isa_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct sbni_isa_softc *isc = (struct sbni_isa_softc *)self;
	struct sbni_softc *sc = &isc->sc_sbni;
	struct isa_attach_args *ia = aux;

	printf("\n");

	/* Map i/o space */
	if (bus_space_map(sc->sc_iot, ia->ia_iobase, SBNI_NPORTS, 0,
	    &sc->sc_ioh)) {
	    	printf("%s: can't map i/o space\n", sc->sc_dev.dv_xname);
		return;
	}

	/* Do bus-independent attach */
	sbni_attach(sc);

	/* Establish the interrupt handler */
	isc->sc_ih = isa_intr_establish(ia->ia_ic, ia->ia_irq, IST_EDGE,
	    IPL_NET, sbni_intr, sc, sc->sc_dev.dv_xname);
	if (isc->sc_ih == NULL)
		printf("%s: couldn't establish interrupt handler\n",
		    self->dv_xname);
	/* What??? sbni_init(sc); */
}

static int
sbni_detect(sc)
	struct sbni_softc *sc;
{
	u_int8_t csr0;

	csr0 = INB(CSR0);
	if (csr0 == 0x00 || csr0 == 0xff)
		return (0);
	csr0 &= ~EN_INT;
	if (csr0 & BU_EMP)
		csr0 |= EN_INT;
	if ((VALID_DECODER & (1 << (csr0 >> 4))) == 0)
		return (0);
	OUTB(CSR0, 0);

	return (1);
}
