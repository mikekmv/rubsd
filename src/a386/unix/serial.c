/**********************************************************************
unix/serial.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file implements a serial-line device.

EXTERNAL INTERFACE

a386_serial_init()		Initialize the serial device.
a386_serial_receive()		Receive a character.
a386_serial_transmit()		Transmit a character.
a386_serial_rx_ready()		Ready to receive a character?
a386_serial_tx_ready()		Ready to transmit a character?
a386_serial_rx_interrupt_enable()	Enable receive interrupts.
a386_serial_rx_interrupt_disable()	Disable receive interrupts.
a386_serial_tx_interrupt_enable()	Enable transmit interrupts.
a386_serial_tx_interrupt_disable()	Disable transmit interrupts.

INTERNAL INTERFACE

_a386_serial_poll()		Poll the device.

TODO

Implement a FIFO, i.e. read more than one charater at a time and store
them in an array.

Implement multiple serial ports.
**********************************************************************/

#include <fcntl.h>
#include <unistd.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

#define DEBUG_ON a386_debug_serial
#define DEBUG_NAME "serial"
#define DEBUG_DEFAULT 0
#include "debug_stub.h"

struct
{
  int in, out; /* file descriptors */
  char ch;
  int rx_ready, tx_ready;
  int rx_interrupt_enable;
  int tx_interrupt_enable;
} serial_port;

void
a386_serial_init (void)
{
  int flags;

  serial_port.in = 0;
  serial_port.in = 1;
  serial_port.rx_ready = 0;
  serial_port.tx_ready = 1;
  serial_port.rx_interrupt_enable = 0;
  serial_port.tx_interrupt_enable = 0;

  flags = unix_fcntl (serial_port.in, F_GETFL, 0);
  unix_fcntl (serial_port.out, F_SETFL, flags | O_NONBLOCK);
  _a386_terminal_raw (serial_port.in);

  flags = unix_fcntl (serial_port.out, F_GETFL, 0);
  unix_fcntl (serial_port.out, F_SETFL, flags | O_NONBLOCK);
  _a386_terminal_raw (serial_port.out);
}

int
a386_serial_rx_ready (void)
{
  if (!serial_port.rx_ready)
    {
      int err;

      err = unix_read (serial_port.in, &serial_port.ch, 1);
      if (err == 1)
	serial_port.rx_ready = 1;
      else
	{
	  /* FIXME: parity error? ovverrun? */
	}
    }

  return serial_port.rx_ready;
}

int
a386_serial_tx_ready (void)
{
  return serial_port.tx_ready;
}

void
a386_serial_rx_interrupt_enable (void)
{
  debug ("enable receive interrupts");
  serial_port.rx_interrupt_enable = 1;
}

void
a386_serial_rx_interrupt_disable (void)
{
  debug ("disable receive interrupts");
  serial_port.rx_interrupt_enable = 0;
}

void
a386_serial_tx_interrupt_enable (void)
{
  debug ("enable transmit interrupts");
  serial_port.tx_interrupt_enable = 1;
}

void
a386_serial_tx_interrupt_disable (void)
{
  debug ("disable transmit interrupts");
  serial_port.tx_interrupt_enable = 0;
}

static const char *
display_char (char c)
{
  static char str[5];
  switch (c)
    {
    case '\a': return "\\a";
    case '\b': return "\\b";
    case '\f': return "\\f";
    case '\n': return "\\n";
    case '\r': return "\\r";
    case '\t': return "\\t";
    case 127:  return "\\x7f";
    default:
      if (c < 32)
	{
	  static char *hex = "0123456789abcdef";
	  str[0] = '\\';
	  str[1] = 'x';
	  str[2] = hex[c >> 4];
	  str[3] = hex[c & 0xf];
	  str[4] = 0;
	}
      else
	str[0] = c; str[1] = 0;
      return str;
    }
}

char
a386_serial_receive (void)
{
  a386_serial_rx_ready ();
  serial_port.rx_ready = 0;
  debug ("receive %02x = %s",
	 (unsigned char)serial_port.ch, display_char (serial_port.ch));
  return serial_port.ch;
}

void
a386_serial_transmit (char c)
{
  debug ("transmit %02x = %s", (unsigned char)c, display_char (c));
  unix_write (serial_port.out, &c, 1);
}

void
_a386_serial_poll (void)
{
  if ((a386_serial_rx_ready () &&
       serial_port.rx_interrupt_enable) ||
      (a386_serial_tx_ready () &&
       serial_port.tx_interrupt_enable))
    {
      debug ("interrupt");
      _a386_vectors->serial_interrupt ();
    }
}
