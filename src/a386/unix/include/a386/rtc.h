/**********************************************************************
unix/include/a386/rtc.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file defines the interface to an abstract real-time clock.

a386_rtc_(get|set): manipulate real-time clock value.
**********************************************************************/

#ifndef _A386_RTC_H
#define _A386_RTC_H

extern long a386_rtc_get (void);
extern void a386_rtc_set (long time);

#endif /* _A386_RTC_H */
