/* $RuOBSD: pcmax.c,v 1.4 2003/11/18 16:52:36 tm Exp $ */

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

#define PCMAX_I2C_DELAY	(12000)

struct cfdriver pcmax_cd = {
	NULL, "pcmax", DV_DULL
};

int             pcmax_get_info(void *, struct radio_info *);
int             pcmax_set_info(void *, struct radio_info *);
int		pcmax_get_info(void *, struct radio_info *);
int		pcmax_set_info(void *, struct radio_info *);

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

u_int8_t 
pcmax_read_power(struct pcmax_softc * sc)
{	
	return 
	PCMAX_PORTVAL_TO_POWER(inb(io_port));
}

u_int8_t 
pcmax_get_sda()
{
	/* I dunno why the inbound SDA is bit 6 */
	return (inb(io_port) & 0x20) >> 5;
}
