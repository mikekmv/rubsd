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

#ifndef _DEV_IC_SBNIREG_H_
#define	_DEV_IC_SBNIREG_H_

#define SBNI_NPORTS			4

#define CSR0				0
#define CSR1				1
#define DAT				2

#define BU_EMP				0x02
#define RC_CHK				0x04
#define CT_ZER				0x08
#define TR_REQ				0x10
#define TR_RDY				0x20
#define EN_INT				0x40
#define RC_RDY				0x80

#define PR_RES				0x80

#define VALID_DECODER			0x03ba

#define FRAME_ACK_MASK			(u_int16_t)0x7000
#define FRAME_LEN_MASK			(u_int16_t)0x03FF
#define FRAME_FIRST			(u_int16_t)0x8000
#define FRAME_RETRY			(u_int16_t)0x0800

#define FRAME_SENT_BAD			(u_int16_t)0x4000
#define FRAME_SENT_OK			(u_int16_t)0x3000

#define FL_WAIT_ACK			1
#define FL_NEED_RESEND			2
#define FL_PREV_OK			4
#define FL_SLOW_MODE			8

#define ETHER_MIN_LEN			64
#define ETHER_MAX_LEN			1518
#define	SBNI_MIN_LEN			(ETHER_MIN_LEN - 4)
#define SBNI_MAX_FRAME			1023

#define TR_ERROR_COUNT			32

#define SBNI_SIG			0x5a

#define CRC32(c,crc)			(crc32tab[((size_t)(crc) ^ (c)) & \
					0xff] ^ (((crc) >> 8) & 0x00FFFFFF))
#define CRC32_REMAINDER			0x2144DF1C
#define CRC32_INITIAL			0x00000000

#define DEFAULT_RATE			0
#define DEFAULT_FRAME_LEN		1012

#define CHANGE_LEVEL_START_TICKS	4
#define SBNI_HZ				18

#define DEF_RXL_DELTA			-1
#define DEF_RXL				15

#define INB(offset) bus_space_read_1(sc->sc_iot, sc->sc_ioh, (offset))
#define OUTB(offset, value) bus_space_write_1(sc->sc_iot, sc->sc_ioh, \
 	(offset), (value))	
#define INSB(offset, addr, count) bus_space_read_multi_1( \
	sc->sc_iot, sc->sc_ioh, (offset), (addr), (count))
#define OUTSB(offset, addr, count) bus_space_write_multi_1( \
	sc->sc_iot, sc->sc_ioh, (offset), (addr), (count))

#endif /* _DEV_IC_SBNIREG_H_ */
