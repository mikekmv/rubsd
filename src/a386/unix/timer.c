/**********************************************************************
unix/timer.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file implements the timer operations.

EXTERNAL INTERFACE

A386_TIMER_MAX_HZ		Maximum timer interrupt frequency.
a386_timer_set_hz()		Set timer interrupt frequency.
a386_timer_get_hz()		Get timer interrupt frequency.
a386_timer_start()		Start timer.
a386_timer_stop()		Stop timer.
**********************************************************************/

#include <sys/time.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

static int timer_hz = 0;

void
a386_timer_set_hz (int hz)
{
  _a386_assert_kernel_mode ();

  if (hz <= A386_TIMER_MAX_HZ)
    timer_hz = hz;
  else
    timer_hz = A386_TIMER_MAX_HZ;
}

int
a386_timer_get_hz (void)
{
  _a386_assert_kernel_mode_return (int);
  return timer_hz;
}

void
a386_timer_start (void)
{
  struct itimerval value;
  long microseconds;

  _a386_assert_kernel_mode ();

  if (timer_hz <= 0)
    return;

  microseconds = (1000000 + timer_hz / 2) / timer_hz;

  value.it_interval.tv_sec = microseconds / 1000000;
  value.it_interval.tv_usec = microseconds % 1000000;
  value.it_value = value.it_interval;
  unix_setitimer (ITIMER_VIRTUAL, &value, NULL);
}

void
a386_timer_stop (void)
{
  struct itimerval value;

  _a386_assert_kernel_mode ();

  value.it_interval.tv_sec = 0;
  value.it_interval.tv_usec = 0;
  value.it_value.tv_sec = 0;
  value.it_value.tv_usec = 0;
  unix_setitimer (ITIMER_VIRTUAL, &value, NULL);
}
