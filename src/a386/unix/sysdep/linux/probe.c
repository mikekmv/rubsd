/**********************************************************************
unix/os/probe.c

Copyright (C) 1999 by Lars Brinkhoff.  See the file COPYING for
licensing terms and conditions.

Probes operating-system headers for interesting values.
**********************************************************************/

#include <stdio.h>

#include <fcntl.h> /* O_RDWR */
#include <unistd.h> /* SEEK_SET */
#include <signal.h> /* struct sigcontext */
#include <termios.h> /* struct termios */
#include <sys/ptrace.h> /* struct pt_regs */
#include <asm/ptrace.h>

#define max(a,b) ((a) > (b) ? (a) : (b))

int
main (int argc, char **argv)
{
  printf ("/* Automatically generated file - do not edit. */\n");
  printf ("#define _A386_SIZEOF_REGS %d\n", sizeof (struct pt_regs));
  printf ("#define SIZEOF_TERMINAL_STATE %d\n", sizeof (struct termios));
  return 0;
}
