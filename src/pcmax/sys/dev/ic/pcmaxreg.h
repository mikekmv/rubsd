/* $RuOBSD: pcmaxreg.h,v 1.3 2003/11/26 23:21:11 tm Exp $ */

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

#ifndef _DEV_IC_PCMAXREG_H
#define _DEV_IC_PCMAXREG_H

#define PCMAX_ISA_POWER_MASK 		(0x1c)
#define PCMAX_ISA_SDA_MASK		(0x01)
#define PCMAX_ISA_SCL_MASK		(0x02)
#define PCMAX_PCI_CONTROL_OFFSET	(0x02)
#define PCMAX_PCI_RF_MASK		(0x03)
#define PCMAX_PCI_SDA_MASK		(0x04)
#define PCMAX_PCI_SCL_MASK		(0x08)
#define PCMAX_PCI_I2C_MASK		(0x0c)
#define PCMAX_PCI_I2C_OFFSET		(0x03)
#define PCMAX_I2C_DELAY			(10000)
#define PCMAX_FREQ_STEP			(50000)		
#define PCMAX_POWER_2_PORTVAL(P)	(((~P << 2) & PCMAX_ISA_POWER_MASK) + 1)
#define PCMAX_PORTVAL_2_POWER(V)	((~V & PCMAX_ISA_POWER_MASK) >> 2)
				
#endif /* _DEV_IC_PCMAXREG_H */
