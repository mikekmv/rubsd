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
