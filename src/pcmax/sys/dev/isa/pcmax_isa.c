/* $RuOBSD: pcmax_isa.c,v 1.6 2003/11/26 21:38:34 tm Exp $ */

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

#include <machine/bus.h>

#include <dev/ic/tiger320.h>
#include <dev/isa/isavar.h>
#include <dev/isa/pcmaxreg.h>
#include <dev/isa/pcmaxvar.h>

int	pcmax_isa_match(struct device *, void *, void *);
void	pcmax_isa_attach(struct device *, struct device *, void *);

struct cfattach pcmax_isa_ca = {
	sizeof(struct pcmax_softc), pcmax_isa_match, pcmax_isa_attach
};

void		pcmax_isa_set_scl(struct pcmax_softc *);
void		pcmax_isa_clr_scl(struct pcmax_softc *);
void		pcmax_isa_set_sda(struct pcmax_softc *);
void		pcmax_isa_clr_sda(struct pcmax_softc *);
void		pcmax_isa_write_power(struct pcmax_softc *, u_int32_t);
u_int8_t	pcmax_isa_read_power(struct pcmax_softc *);

int
pcmax_isa_match(struct device *parent, void *match, void *aux)
{
	struct isa_attach_args *ia = aux;
	bus_space_handle_t ioh;
	int iosize = 1;

	if (bus_space_map(ia->ia_iot, ia->ia_iobase, iosize, 0, &ioh))
		return (0);

	bus_space_unmap(ia->ia_iot, ioh, iosize);
	ia->ia_iosize = iosize;
	return (1);
}

void
pcmax_isa_attach(struct device *parent, struct device *self, void *aux)
{
	struct pcmax_softc *sc = (void *) self;
	struct isa_attach_args *ia = aux;
	bus_space_handle_t ioh;

	if (bus_space_map(ia->ia_iot, ia->ia_iobase, ia->ia_iosize, 0, &ioh)) {
		printf(": can't map I/O space\n");
		return;
	}

	sc->set_scl = pcmax_isa_set_scl;
	sc->clr_scl = pcmax_isa_clr_scl;
	sc->set_sda = pcmax_isa_set_sda;
	sc->clr_sda = pcmax_isa_clr_sda;	
	sc->write_power = pcmax_isa_write_power;
	printf(": Pcmax Ultra\n");
	pcmax_attach(sc);
}

void
pcmax_set_scl_isa(struct pcmax_softc * sc)
{
	sc->io_val |= PCMAX_ISA_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->ioval);
}

void
pcmax_clr_scl_isa(struct pcmax_softc * sc)
{
	sc->io_val &= ~PCMAX_ISA_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->ioval);
}

void
pcmax_set_sda_isa(struct pcmax_softc * sc)
{
	sc->io_val |= PCMAX_ISA_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->ioval);
}

void
pcmax_clr_sda_isa(struct pcmax_softc * sc)
{
	sc->io_val &= ~PCMAX_ISA_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->ioval);
}

void
pcmax_isa_write_power(struct pcmax_softc * sc, u_int32_t p)
{
	sc->iov = PCMAX_POWER_2_PORTVAL(p) | (sc->iov & (~PCMAX_ISA_POWER_MASK));
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, 0, sc->ioval);
}

u_int8_t
pcmax_isa_read_power(struct pcmax_softc * sc)
{
	u_int8_t p;
	p = bus_space_read_1(sc->sc_iot, sc->sc_ioh, 0);
	return (PCMAX_PORTVAL_2_POWER(p));
}
