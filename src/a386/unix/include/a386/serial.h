/**********************************************************************
a386.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file defines the interface to an abstract serial port.

a386_serial_init: initialize.
a386_serial_(status|set_mode): device control.
a386_serial_(receive|transmit): receive/transmit character.
a386_serial_(rx|tx)_ready: ready to receive a character?
a386_serial_(rx|tx)_interrupt_(enable|disable): enable/disable rx/tx interrupts
**********************************************************************/

#ifndef _A386_SERIAL_H
#define _A386_SERIAL_H

extern void a386_serial_init (void);
extern char a386_serial_receive (void);
extern void a386_serial_transmit (char c);
extern int a386_serial_rx_ready (void);
extern int a386_serial_tx_ready (void);
extern void a386_serial_rx_interrupt_enable (void);
extern void a386_serial_rx_interrupt_disable (void);
extern void a386_serial_tx_interrupt_enable (void);
extern void a386_serial_tx_interrupt_disable (void);

#endif /* _A386_SERIAL_H */
