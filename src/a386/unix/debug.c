/**********************************************************************
unix/debug.c

Copyright (C) 1999-2000 Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

Debugging utilities.

EXTERNAL INTERFACE
a386_putchar()			Print a raw character to stdout.
a386_putchar_cooked()		Print a cooked character to stdout.
a386_getchar()			Get a character from stdin.
a386_puts()			Print a string.
a386_printf()			Print a formatted string.
a386_dump_stack()		Print stack contents.
**********************************************************************/

#include "sysdep/os/syscalls.h"

#include <a386/a386_types.h> /* before a386_reg.h! */
#include <a386/a386_reg.h>

#include <a386/a386_debug.h>

void
a386_putchar (char c)
{
  unix_write (2, &c, 1);
}

int
a386_getchar (void)
{
  unsigned char c;
  unix_read (0, &c, 1);
  return (int)c;
}

void
a386_putchar_cooked (char c)
{
  if (c == '\n')
    a386_putchar ('\r');
  a386_putchar (c);
}

void
a386_puts (const char *s)
{
  while (*s)
    a386_putchar_cooked (*s++);
}

static void
_a386_write_num (int x, int base, int width, int zero)
{
  char *hex = "0123456789abcdef";
  unsigned int y, b = base;
  char buf[100];
  int neg = 0;
  int n = 0;

  y = x;
  if (x < 0 && base == 10)
    {
      y = -x;
      neg = 1;
    }

  if (y == 0)
    buf[n++] = '0';
  else
    while (y > 0)
      {
	buf[n++] = hex[y % b];
	y /= b;
      }

  if (n < width)
    {
      int i;
      char c = zero ? '0' : ' ';
      for (i = 0; i < width - n; i++)
	a386_putchar (c);
    }
  if (neg)
    a386_putchar ('-');
  while (n > 0)
    a386_putchar (buf[--n]);
}

static inline void
_a386_write_dec (int x, int width, int zero)
{
  _a386_write_num (x, 10, width, zero);
}

static inline void
_a386_write_hex (int x, int width, int zero)
{
  _a386_write_num (x, 16, width, zero);
}

void
a386_printf (const char *format, ...)
{
  const char *p;
  const char *arg = (char *)&format + sizeof (char *);

  for (p = format; *p; p++)
    {
      if (*p == '%')
	{
	  int done = 0, zero = 0, width = 0, loong = 0;
	  while (!done)
	    {
	      p++;
	      switch (*p)
		{
		case '\0':
		  a386_putchar ('%');
		  done = 1;
		  return;
		case 'l':
		  loong = 1;
		  break;
		case 'c':
		  a386_putchar_cooked ((char)*(int *)arg);
		  arg += sizeof (int);
		  done = 1;
		  break;
		case 'u':
		case 'd':
		  _a386_write_dec (*(int *)arg, width, zero);
		  arg += sizeof (int);
		  done = 1;
		  break;
		case 'p':
		  width = 2 * sizeof (long);
		  loong = 1;
		  zero = 1;
		  a386_putchar ('0');
		  a386_putchar ('x');
		  /* fall through */
		case 'x':
		  _a386_write_hex (*(int *)arg, width, zero);
		  arg += sizeof (int);
		  done = 1;
		  break;
		case 's':
		  a386_puts (*(char **)arg);
		  arg += sizeof (char *);
		  done = 1;
		  break;
		case '0':
		  if (zero == 0)
		    zero = 1;
		  else
		    width *= 10;
		  break;
		case '1': case '2': case '3': case '4': case '5': 
		case '6': case '7': case '8': case '9':
		  width = 10 * width + *p - '0';
		  break;
		default:
		  a386_putchar_cooked (*p);
		  done = 1;
		}
	    }
	}
      else
	{
	  a386_putchar_cooked (*p);
	}
    }
}
