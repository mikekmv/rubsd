/**********************************************************************
unix/rtc.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file implementes a real-time clock.

EXTERNAL INTERFACE

a386_rtc_get()			Get the value of the RTC.
a386_rtc_set()			Set the value of the RTC.
**********************************************************************/

#include <sys/time.h>

#include "_a386.h"
#include "sysdep/os/syscalls.h"

long
a386_rtc_get (void)
{
  struct timeval tv;

  _a386_assert_kernel_mode_return (long);
  unix_gettimeofday (&tv, NULL);
  return tv.tv_sec;
}

void
a386_rtc_set (long time)
{
  struct timeval tv;
  
  _a386_assert_kernel_mode ();
  tv.tv_sec = time;
  tv.tv_usec = 0;
  unix_settimeofday (&tv, NULL);
}
