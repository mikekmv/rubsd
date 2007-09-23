/* $oganer: timer.c,v 1.4 2002/08/07 12:39:44 tm Exp $ */
#include <sys/time.h>
#include <timer.h>
#include <syslog.h>

/* 
 * Set new timer value (current time increased with given period)
 */

// Do not increase at all (useful for counts, not for timeouts)
int tm_zero (timeval_t * tm)
{  
   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_zero(): NULL argument");
      return TIMER_ERROR;
   }

   if (gettimeofday(tm, NULL) != 0)
   {  syslog(LOG_ERR, "tm_zero(gettimeofday): %m");
      return TIMER_ERROR;
   }

   return 0;
}

// Set interval in both secons & useconds (basic function)
int tm_setsu (timeval_t * tm, int sec, int usec)
{  
   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_setsu(): NULL argument");
      return TIMER_ERROR;
   }

// Get current time
   if (gettimeofday(tm, NULL) != 0)
   {  syslog(LOG_ERR, "tm_setsu(gettimeofday): %m");
      return TIMER_ERROR;
   }

// Increase current time to given period
// increase useconds
   tm->tv_usec += usec%1000000;
// add seconds, useconds overflow, and posible usec arg overflow
   tm->tv_sec  += sec + tm->tv_usec/1000000 + usec/1000000 ;
// correct useconds
   tm->tv_usec %= 1000000;

   return 0;   
}

// Set interval in seconds only
int tm_sets (timeval_t * tm, int sec)
{  
   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_sets(): NULL argument");
      return TIMER_ERROR;
   }

   return tm_setsu(tm, sec, 0);
}

// Set interval on useconds only
int tm_setu (timeval_t * tm, int usec)
{  
   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_setu(): NULL argument");
      return TIMER_ERROR;
   }

   return tm_setsu(tm, 0, usec);
}

// Set interval from timeval structure
int tm_set (timeval_t * tm, timeval_t * per)
{  
   if (tm == NULL || per == NULL) 
   {  syslog(LOG_ERR, "tm_set(%p, %p): NULL argument", tm, per);
      return TIMER_ERROR;
   }

   return tm_setsu(tm, per->tv_sec, per->tv_usec);
}

/* 
 *  Return timer state (FALSE if reached)
 *   (actually, checks is current time equal or greater
 *     than timer value)
 */

int  tm_state (timeval_t * tm)
{  timeval_t       tv;

   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_state(): NULL argument");
      return TIMER_ERROR;
   }

   if (gettimeofday(&tv, NULL) != 0)
   {  syslog(LOG_ERR, "tm_state(gettimeofday): %m");
      return TIMER_ERROR;
   }

   if (tm->tv_sec  > tv.tv_sec)  return TRUE;  // seconds less
   if (tm->tv_sec  < tv.tv_sec)  return FALSE; // seconds greater
   if (tm->tv_usec > tv.tv_usec) return TRUE;  // seconds equal, usec less

   return FALSE;
}


/* 
 * Return time remain before reach timer
 */

// Return remainig time on timeval (basic function)
int  tm_rem (timeval_t * tm, timeval_t * dest)
{  timeval_t curr;

   if (tm == NULL || dest == NULL) 
   {  syslog(LOG_ERR, "tm_rem(%p, %p): NULL argument", tm, dest);
      return TIMER_ERROR;
   }

   if (gettimeofday(&curr, NULL) != 0)
   {  syslog(LOG_ERR, "tm_rem(gettimeofday): %m");
      return TIMER_ERROR;
   }

// Underflow if tm is less than current
   if (tm->tv_sec < curr.tv_sec ||
        (tm->tv_sec == curr.tv_sec && tm->tv_usec < curr.tv_usec)
      ) return TIMER_OVERFLOW;

// Action - dest = tm - current

   *dest = *tm;
   if (dest->tv_usec < curr.tv_usec)
   {  dest->tv_usec += 1000000;
      dest->tv_sec  -= 1;
   }
   dest->tv_sec  -= curr.tv_sec; 
   dest->tv_usec -= curr.tv_usec; 

   return 0;
}

// Return remainig time on useconds
int  tm_remu (timeval_t * tm)
{  timeval_t   rem;
   int         rc;

   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_remu(): NULL argument");
      return TIMER_ERROR;
   }
   
   rc = tm_rem(tm, &rem);
   if (rc != 0) return rc;
   if (rem.tv_sec > 0) return TIMER_SECONDS;

   return rem.tv_usec;
}

// Return remainig time on seconds
int  tm_rems (timeval_t * tm)
{  timeval_t   rem;
   int         rc;

   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_rems(): NULL argument");
      return TIMER_ERROR;
   }
   
   rc = tm_rem(tm, &rem);
   if (rc != 0) return rc;

   return rem.tv_sec;
}

/*
 *  Return time passed AFTER timer value
 */

// Return time after timer on timeval (basic function)
int  tm_left (timeval_t * tm, timeval_t * dest)
{  timeval_t curr;

   if (tm == NULL || dest == NULL) 
   {  syslog(LOG_ERR, "tm_left(%p, %p): NULL argument", tm, dest);
      return TIMER_ERROR;
   }

   if (gettimeofday(&curr, NULL) != 0)
   {  syslog(LOG_ERR, "tm_left(gettimeofday): %m");
      return TIMER_ERROR;
   }

// Underflow if tm is greater than current
   if (tm->tv_sec > curr.tv_sec ||
        (tm->tv_sec == curr.tv_sec && tm->tv_usec > curr.tv_usec)
      ) return TIMER_OVERFLOW;

// Action - dest = current - tm

   *dest = curr;
   if (tm->tv_usec > curr.tv_usec)
   {  dest->tv_usec += 1000000;
      dest->tv_sec  -= 1;
   }
   dest->tv_sec  -= tm->tv_sec; 
   dest->tv_usec -= tm->tv_usec; 

   return 0;
}

// Return useconds only
int  tm_leftu (timeval_t * tm)
{  timeval_t   left;
   int         rc;

   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_leftu(): NULL argument");
      return TIMER_ERROR;
   }
   
   rc = tm_left(tm, &left);
   if (rc != 0) return rc;
   if (left.tv_sec > 0) return TIMER_SECONDS;

   return left.tv_usec;
}

// Return seconds only
int  tm_lefts (timeval_t * tm)
{  timeval_t   left;
   int         rc;

   if (tm == NULL) 
   {  syslog(LOG_ERR, "tm_lefts(): NULL argument");
      return TIMER_ERROR;
   }
   
   rc = tm_left(tm, &left);
   if (rc != 0) return rc;

   return left.tv_sec;
}
