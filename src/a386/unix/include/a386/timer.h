/**********************************************************************
a386.h

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.

This file defines the interface to an abstract timer.

a386_timer_(get|set)_hz: manipulte timer frequency.
a386_timer_(start|stop): start/stop timer.
**********************************************************************/

#ifndef _A386_TIMER_H
#define _A386_TIMER_H

void a386_timer_set_hz (int hz);
int a386_timer_get_hz (void);
void a386_timer_start (void);
void a386_timer_stop (void);

#endif /* _A386_TIMER_H */
