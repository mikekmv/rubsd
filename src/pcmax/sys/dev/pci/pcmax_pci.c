/* $RuOBSD: pcmax.c,v 1.3 2003/11/17 15:19:11 tm Exp $ */

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

#define PCMAX_PCI_RF_MASK		(0x03)
#define PCMAX_PCI_CONTROL_OFFSET	(0x02)

#define	PCMAX_PCI_CBIO			(0x10)	/* I/O map BAR */
#define PCMAX_PCI_CMEM			(0x14)	/* memory map BAR */		

struct radio_hw_if pcmax_hw_if = {
	NULL,			/* open */
	NULL,			/* close */
	pcmax_get_info,
	pcmax_set_info,
	NULL,			/* search */
};

struct pcmax_softc {
	struct device   sc_dev;
	bus_space_tag_t	iot;
	bus_space_handle_t ioh;
         
	int             mute;
	u_int8_t        vol;
	u_int8_t	ioc;
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


void	pcmax_set_mute(struct pcmax_softc *);
void	i2c_delay(void);

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
	struct pci_attach_args *pa = aux;
	pci_chipset_tag_t pc = pa->pa_pc;
	pcireg_t csr;
        in rv;
                                
	if (!sc->sc_dev.dv_cfdata->cf_flags) {	/* I/O */
		rv = pci_mapreg_map(pa, PCMAX_PCI_CBIO, PCI_MAPREG_TYPE_IO,
			0, &sc->sc_iot, &sc->sc_ioh, NULL, NULL, 0);
	} else {				/* memory */
		rv = pci_mapreg_map(pa, PCMAX_PCI_CMEM, PCI_MAPREG_TYPE_MEM,
			0, &sc->sc_iot, &sc->sc_ioh, NULL, NULL, 0);
	}

	if (rv) {
		printf(": can't map i/o space\n");
		return;
	}

	/* Enable the card */
	csr = pci_conf_read(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
		csr | PCI_COMMAND_MASTER_ENABLE);

	sc->mute = 0;
	sc->ioc = 0;

	/* enable I2C */
	sc->ioc |= PCMAX_PCI_I2C_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_CONTROL_OFFSET, sc->ioc);
	outb(io_control, PCMAX_PCI_CONTROL_PORT(io_port));
	
	printf(": Pcimax Ultra FM-Transmitter\n");
	pcmax_write_power(sc);
	radio_attach_mi(&pcmax_hw_if, sc, &sc->sc_dev);
}

int
pcmax_get_info(void *v, struct radio_info * ri)
{
	return (0);
}

int
pcmax_set_info(void *v, struct radio_info * ri)
{
	return (0);
}

void
pcmax_set_mute(struct pcmax_softc * sc)
{
	if (!sc->mute)
		sc->ioc &= ~PCMAX_PCI_RF_MASK;
	else
		sc->ioc |= PCMAX_PCI_RF_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_CONTROL_OFFSET, sc->ioc);
}

void
i2c_delay(void)
{
	DELAY(10000);	
}

/* Set the SDA, pulling it high */
void
set_sda_pci(struct pcmax_softc * sc)
{
	sc->iov |= PCMAX_PCI_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET, sc->iov);
}

/* Set the SCL, pulling it high*/
void 
set_scl_pci(struct pcmax_softc * sc)
{
	sc->iov |= PCMAX_PCI_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET, sc->iov);
}

/* clear the SDA bit */
void clr_sda_pci(struct pcmax_softc * sc)
{
	sc->iov &= ~PCMAX_PCI_SDA_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET, sc->iov);
}

/* Clear the SCL bit */
void
clr_scl_pci(struct pcmax_softc * sc)
{
	sc->iov &= ~PCMAX_PCI_SCL_MASK;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, PCMAX_PCI_I2C_OFFSET, sc->iov);
}

/* Sends the I2C start sequence of SDA, SCL -> Low*/
void
i2c_start(struct pcmax_softc * sc)
{
	/* first bring them high */
	set_sda(sc);
	set_scl(sc);
	
	/* Then the i2c start sequence */
	i2c_delay();
	clr_sda(sc);
	i2c_delay();
	clr_scl(sc);
	i2c_delay();
}

/* Sends the i2c stop sequence */
void
i2c_stop(struct pcmax_softc * sc)
{
	/* bring sda low just in case */
	clr_sda(sc);
	i2c_delay();
	set_scl(sc);
	i2c_delay();
	set_sda(sc);
}

/* Writes a single bit to i2c */
void
i2c_write_bit(struct pcmax_softc * sc, int bit)
{
	if(bit)
		set_sda(sc);
	else
		clr_sda(sc);

	/* end the bit transmission with a tick of the SCL clock */
	i2c_delay();
	set_scl(sc);
	i2c_delay();
	clr_scl(sc);
	i2c_delay();
}

/* you have to call i2c_start before using this function */
void 
i2c_write_byte(struct pcmax_softc * sc, u_int8_t val)
{
	int i;

	for(i = 0; i < sizeof(val) << 3; ++i)
	{
		/* output the MSB first: 
		 * Byte & 0x80 gets the highest bit, and we shift i over
		 * to get successively lower bits */
		i2c_write_bit(sc, (val << i) & 0x80);
	}
	/* not sure what this does.. looks like it clears the byte */
	set_sda(sc);
	i2c_delay();
	set_scl(sc);
	i2c_delay();
	clr_scl(sc);
	i2c_delay();
}

/* Write a frequency to the PLL on the FM TX card */
void
i2c_write_pll(struct pcmax_softc * sc, int freq)
{
	i2c_delay();
	i2c_start(sc);

	freq_steps = freq;

	/* send the i2c address */
	i2c_write_byte(192); 

	/* Send the msb of the freq first (byte 1) */
	i2c_write_byte(sc, (freq & 0xff00)>> 8);

	/* then the lsb  (byte 2) */
	i2c_write_byte(sc, (freq & 0x00ff));

	/* Send the ?? (byte 3) */
	i2c_write_byte(sc, 142);

	/* Then byte 4 */
	i2c_write_byte(sc, 0);

	i2c_stop(sc);
}

u_int8_t 
pcmax_read_power()
{
	return PCMAX_PORTVAL_TO_POWER(inb(io_port));
}

static u8 get_sda()
{
	/* I dunno why the inbound SDA is bit 6 */
	return (inb(io_port) & 0x20) >> 5;
}

u_int8_t 
i2c_read_byte(struct pcmax_softc * sc)
{
	u_int8_t rv = 0;
	int i;

	i2c_delay();

	/* This appears to come in in LSB first order */
	for(i = 0; i < 8; i++)
	{
		i2c_delay();
		/* set scl high to read */
		set_scl(sc);
		i2c_delay();

		/* build the result, LSB first */
		rv |= get_sda(sc) << i;

		clr_scl(sc);
	}

	i2c_delay();
	set_sda(sc);
	i2c_delay();
	set_scl(sc);
	i2c_delay();
	clr_scl(sc);

	return (rv);
}

int 
i2c_read_pll(struct pcmax_softc * sc)
{
	int status = 0;

	i2c_delay();
	i2c_start(sc);

	/* Must set SDA on read */
	i2c_write_byte(192 | 1);

	status = i2c_read_byte(sc);

	i2c_stop(sc);

	return (status);
}
