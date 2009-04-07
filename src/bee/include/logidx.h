/* $RuOBSD$ */

#ifndef __LOGIDX_H__
#define __LOGIDX_H__

//
//  Create beelog date index
//
//  Index format:
//  Head:
//     file marker (to prevent incorrect file usage)
//     first record date in UTC
//  Body:
//     index (rec#) for first + 1 day
//     index (rec#) for first + 2 days
//     ....
//     index (rec#) for last day in file
//

#define IDXMARKER "bee1"

typedef struct
{  char     marker[4];
   time_t   first;
} idxhead_t;
#define IDXMARKER "bee1"

#endif /* __LOGIDX_H__ */

