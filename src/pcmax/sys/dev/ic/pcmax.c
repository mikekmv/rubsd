/* $RuOBSD: pcmax.c,v 1.9 2003/11/27 23:25:21 tm Exp $ */

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
#include <sys/radioio.h>
#include <sys/types.h>

#include <machine/bus.h>

#include <dev/radio_if.h>

#include <dev/ic/pcmaxreg.h>
#include <dev/ic/pcmaxvar.h>

int	pcmax_get_info(void *, struct radio_info *);
int	pcmax_set_info(void *, struct radio_info *);

void 	pcmax_i2c_start(struct pcmax_softc *);
void	pcmax_i2c_stop(struct pcmax_softc *);	
void	pcmax_i2c_write_bit(struct pcmax_softc *, int);
void 	pcmax_i2c_write_byte(struct pcmax_softc *, u_int8_t);
u_int8_t pcmax_i2c_read_byte(struct pcmax_softc *);
void	pcmax_i2c_write_pll(struct pcmax_softc *, u_int32_t);
u_int8_t pcmax_i2c_read_pll(struct pcmax_softc *);
u_int8_t pcmax_get_sda(struct pcmax_softc *);

struct radio_hw_if pcmax_hw_if = {
	NULL,			/* open */
	NULL,			/* close */
	pcmax_get_info,
	pcmax_set_info,
	NULL			/* search */
};

struct cfdriver pcmax_cd = {
	NULL, "pcmax", DV_DULL
};

void
pcmax_attach(struct pcmax_softc * sc)
{
	sc->mute = 0;
	sc->vol = 0;
	sc->freq = MAX_FM_FREQ;
	sc->stereo = 1;
	
	radio_attach_mi(&pcmax_hw_if, sc, &sc->sc_dev);
}

int
pcmax_get_info(void *v, struct radio_info * ri)
{
	struct pcmax_softc *sc = v;
	
	ri->mute = sc->mute;
	ri->volume = sc->read_power(sc); 
	ri->stereo = sc->stereo;
	ri->freq = pcmax_i2c_read_pll(sc);
	ri->info = RADIO_INFO_STEREO;	
	ri->rfreq = ri->lock = 0;
	ri->caps = PCMAX_CAPABILITIES;
	return (0);                                        	
}

int
pcmax_set_info(void *v, struct radio_info * ri)
{
	struct pcmax_softc *sc = v;
	
	sc->mute = ri->mute ? 1 : 0;
	if (sc->mute)
		ri->volume = 0;
	sc->vol = ri->volume;
	sc->write_power(sc);
	pcmax_i2c_write_pll(sc, ri->freq);
	return (0);
}

/* sends the I2C start sequence of SDA, SCL -> Low */
void 
pcmax_i2c_start(struct pcmax_softc * sc)
{
	/* first bring them high */
	sc->set_sda(sc);
	sc->set_scl(sc);
	
	/* then the i2c start sequence */
	DELAY(PCMAX_I2C_DELAY);
	sc->clr_sda(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->clr_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
}

/* sends the i2c stop sequence */
void
pcmax_i2c_stop(struct pcmax_softc * sc)
{
	/* bring sda low just in case */
	sc->clr_sda(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->set_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->set_sda(sc);
	DELAY(PCMAX_I2C_DELAY);	/* ? */
}

/* writes a single bit to i2c */
void
pcmax_i2c_write_bit(struct pcmax_softc * sc, int bit)
{
	!bit ? sc->clr_sda(sc) : sc->set_sda(sc);

	/* end the bit transmission with a tick of the SCL clock */
	DELAY(PCMAX_I2C_DELAY);
	sc->set_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->clr_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
}

/* note, you have to call i2c_start before using this function */
void 
pcmax_i2c_write_byte(struct pcmax_softc * sc, u_int8_t v)
{
	int i;

	for (i = 0; i < sizeof(v) << 3; i++) {
		/* output the MSB first: 
		 * byte & 0x80 gets the highest bit, and we shift it over
		 * to get successively lower bits 
		 */
		pcmax_i2c_write_bit(sc, (v << i) & 0x80);
	}
	/* not sure what this does.. looks like it clears the byte */
	sc->set_sda(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->set_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->clr_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
}

/* Write a frequency to the PLL on the FM TX card */
void
pcmax_i2c_write_pll(struct pcmax_softc * sc, u_int32_t freq)
{
	sc->freq = freq/PCMAX_FREQ_STEP;

	DELAY(PCMAX_I2C_DELAY);	
	pcmax_i2c_start(sc);
	
	/* send the i2c address */
	pcmax_i2c_write_byte(sc, 192); 

	/* send the msb of the freq first (byte 1) */
	pcmax_i2c_write_byte(sc, (sc->freq & 0xFF00) >> 8);

	/* then the lsb  (byte 2) */
	pcmax_i2c_write_byte(sc, (freq & 0x00FF));

	/* send byte 3 */
	pcmax_i2c_write_byte(sc, 142);

	/* then byte 4 */
	pcmax_i2c_write_byte(sc, 0);

	pcmax_i2c_stop(sc);
}

u_int8_t
pcmax_i2c_read_pll(struct pcmax_softc * sc)
{
	u_int8_t status = 0;

	DELAY(PCMAX_I2C_DELAY);
	pcmax_i2c_start(sc);

	/* must set SDA on read */
	pcmax_i2c_write_byte(sc, 192 | 1);
	
	status = pcmax_i2c_read_byte(sc);
	pcmax_i2c_stop(sc);

	return (status);
}


u_int8_t 
pcmax_i2c_read_byte(struct pcmax_softc * sc)
{
	int i;
	u_int8_t rv = 0;

	DELAY(PCMAX_I2C_DELAY);	

	/* this appears to come in in LSB first order */
	for(i = 0; i < 8; i++) {
		DELAY(PCMAX_I2C_DELAY);
		
		/* set scl high to read */
		sc->set_scl(sc);
		
		DELAY(PCMAX_I2C_DELAY);

		/* build the result, LSB first */
		rv |= pcmax_get_sda(sc) << i;
		
		sc->clr_scl(sc);
	}
	
	DELAY(PCMAX_I2C_DELAY);
	sc->set_sda(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->set_scl(sc);
	DELAY(PCMAX_I2C_DELAY);
	sc->clr_scl(sc);

	return (rv);
}

u_int8_t 
pcmax_get_sda(struct pcmax_softc * sc)
{
	return ((bus_space_read_1(sc->sc_iot, sc->sc_ioh, 0) & 0x20) >> 5);
}
