/* $oganer: timer.h,v 1.4 2002/08/07 12:39:41 tm Exp $ */
#ifndef __TIMER_H_
#define __TIMER_H_

/*
 *    "timer" on this code is timeval structure, that contains time
 *  value for waited moment or current value some time ago. Ever stored
 *  you can get difference with current time (time before timer value
 *  or after it). There are two task for this code:
 *    >> Wait (non-blocking) for some interval
 *        1. Store current time, increased with given time
 *        2. Periodically check until current time be equal or
 *           greater timer value (with ability to get time remain
 *           for this moment)
 *    >> Timing some action
 *        1. Store current time
 *        2. Perform timed action
 *        3. Get passed time values     
 *     
 */


#include <sys/time.h>

#define TIMER_OK	0
#define TIMER_ERROR	(-1)	/* invalid arguments, errors */
#define TIMER_SECONDS	(-2)	/* unable to return seconds on *u() */
#define TIMER_OVERFLOW	(-3)	/* diff is negative */

#define FALSE		0
#define TRUE		1

typedef struct timeval timeval_t;

// Set new timer value
int  tm_zero    (timeval_t * tm);
int  tm_set     (timeval_t * tm, timeval_t * per);
int  tm_sets    (timeval_t * tm, int sec);
int  tm_setu    (timeval_t * tm, int usec);
int  tm_setsu   (timeval_t * tm, int sec, int usec);

// Check timer state (zero if time is reached)
int  tm_state   (timeval_t * tm);

// Get time remain before timer reaching
int  tm_rem     (timeval_t * tm, timeval_t * dest);
int  tm_remu    (timeval_t * tm);
int  tm_rems    (timeval_t * tm);

// Get time passed after timer reached (timings)
int  tm_left     (timeval_t * tm, timeval_t * dest);
int  tm_leftu    (timeval_t * tm);
int  tm_lefts    (timeval_t * tm);

#endif /*__TIMER_H_*/
