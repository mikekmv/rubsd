/* $RuOBSD: beerep.h,v 1.4 2004/09/11 14:31:31 shadow Exp $*/
#ifndef __LOGINDEX_H__
#define __LOGINDEX_H__

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

typedef struct
{  char     marker[4];
   time_t   first;
} idxhead_t;

#endif /* __LOGINDEX_H__ */
