/* $RuOBSD: res.c,v 1.7 2004/04/20 04:02:17 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <res.h>

int         resourcecnt=4;
resource_t  resource[]=
{  
   {0, inet_count_proc,  "inet","/usr/local/bin/beepfrules.sh", 1},
   {0, mail_count_proc,  "mail",	NULL, 0},
   {0, adder_count_proc, "adder",	NULL, 0},
   {0, intra_count_stub, "intra",       NULL, 0},
};

#define DELIM  " ,\t\n\r"

typedef struct
{  int      hour_from;
   int      hour_to;
   money_t  price;
} inet_tariff_t;

inet_tariff_t  inet_tariffs[] =
{  { 0,  0,  3.0},  // default price
   { 2,  9,  1.8},  // night dead time
   {15, 19,  3.5},  // day rush hour
   {-1, -1, -1  }   // (terminator)
};

money_t inet_count_proc(is_data_t * data)
{  money_t    retval = 0;
   time_t     curtime;
   time_t     stime;
   struct tm  stm;
   int        tariff = 0;  // default tariff
   int        i;

#define CORR_VALUE           300      /* 5 minutes */

// Get current time
   curtime = time(NULL);
   localtime_r(&curtime, &stm);

// Count day start time (0:00)
   stm = stm;
   stm.tm_hour = 0;
   stm.tm_min  = 0;
   stm.tm_sec  = 0;   
   stime  = timelocal(&stm);

// Add realtime time correction
   stime += CORR_VALUE;  

// Find tariff index
   for (i=1; inet_tariffs[i].hour_from >= 0; i++)
   {  if (curtime >= (stime + inet_tariffs[i].hour_from * 3600) &&
          curtime <  (stime + inet_tariffs[i].hour_to   * 3600))  tariff = i;
   }
   
   retval = -((money_t)data->value * inet_tariffs[tariff].price /(1024*1024));

   return retval;
}

money_t mail_count_proc(is_data_t * data)
{
   return 0; // Mail is for free ;)
}

money_t adder_count_proc(is_data_t * data)
{
   return (money_t)data->value/100; 
}

money_t intra_count_stub(is_data_t * data)
{
   return 0; // Intranet is manual only
}

