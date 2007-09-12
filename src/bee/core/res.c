/* $RuOBSD: res.c,v 1.17 2007/09/12 09:53:24 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <db.h>
#include <res.h>
#include <tariffs.h>

int         resourcecnt = 5;

resource_t  resource[]=
{  
   {0, inet_count_proc,    inet_charge_proc,   "inet",	"/usr/local/bin/beepfrules.sh", 1},
   {0, mail_count_proc,                NULL,   "mail",	NULL, 0},
   {0, adder_count_proc,               NULL,  "adder",	NULL, 0},
   {0, intra_count_stub,               NULL,  "intra",	NULL, 0},
   {0, charge_count_proc,              NULL, "charge",	NULL, 0},
};

#define DELIM  " ,\t\n\r"

money_t inet_count_proc(is_data_t * data, acc_t * acc)
{  money_t    val = 0;
   time_t     curtime;
   time_t     stime;
   struct tm  stm;
   int        def_tariff = 0;  // set to global default
   int        tariff     = 0;  // set to global default
   int        i;

#define CORR_VALUE           300      /* 5 minutes */

// Get current time
   curtime = time(NULL);

// Summ traffic & money
// check reset time (& reset if reached)
   fprintf(stderr, "DEBUG: curtime = %d\n", curtime);
   fprintf(stderr, "DEBUG: rsttime = %d\n", acc->summ_rsttime);

   if (curtime >= acc->summ_rsttime)
   {

   // reset counters, set new reset time
      acc->inet_summary  = 0;
      acc->money_summary = 0;

   // set new reset time 
      localtime_r(&curtime, &stm);
      stm.tm_mday = 1;
      stm.tm_mon++;
      if (stm.tm_mon > 11)
      {  stm.tm_mon = 0;
         stm.tm_year++;
      }
      stm.tm_hour = 0;
      stm.tm_min  = 0;
      stm.tm_sec  = 0;

      stime = timelocal(&stm);
      if (stime < 0)
         syslog(LOG_ERR, "inet_count_proc(timelocal(%d:%d:%d)): cannot assemble time",
                stm.tm_mday, stm.tm_mon+1, stm.tm_year+1900);
      else
         acc->summ_rsttime = stime;        
   }
// add current values to summ values
   acc->inet_summary  += data->value;

// Break-down current time to values
   localtime_r(&curtime, &stm);

// Find default tariff (for account tariff number)
// (last matching wins)
   for (i=0; inet_tariffs[i].hour_from >= 0; i++)
   {  if (acc->tariff == inet_tariffs[i].tariff &&
          (inet_tariffs[i].weekday < 0 || inet_tariffs[i].weekday == stm.tm_wday) &&
          inet_tariffs[i].hour_from == 0           &&
          inet_tariffs[i].hour_to   == 0) def_tariff = i;
   }

// Set tariff to default
   tariff = def_tariff;

// Count day start time (0:00)
   stm = stm;
   stm.tm_hour = 0;
   stm.tm_min  = 0;
   stm.tm_sec  = 0;   
   stime  = timelocal(&stm);

// Add realtime time correction
   stime += CORR_VALUE;  

// Find tariff index by time & tariff & weekday
// (last matching wins)
   for (i=1; inet_tariffs[i].hour_from >= 0; i++)
   {  if (acc->tariff == inet_tariffs[i].tariff                 &&
          (inet_tariffs[i].weekday < 0 || inet_tariffs[i].weekday == stm.tm_wday) &&
          inet_tariffs[i].hour_from != inet_tariffs[i].hour_to  &&
          curtime >= (stime + inet_tariffs[i].hour_from * 3600) &&
          curtime <  (stime + inet_tariffs[i].hour_to   * 3600))  tariff = i;
   }

// trap default tariff field
   if (inet_tariffs[tariff].price_in < 0) tariff = def_tariff;

// get price value
   if ((data->proto_id & 0x80000000) == 0) 
      val = inet_tariffs[tariff].price_in;
   else 
      val = inet_tariffs[tariff].price_out;

// count transaction sum
   val = (money_t)data->value * val / (1024*1024);

// Summ money value
   acc->money_summary += val;

   return -val;
}

money_t mail_count_proc(is_data_t * data, acc_t * acc)
{
   return 0; // Mail is for free ;)
}

money_t adder_count_proc(is_data_t * data, acc_t * acc)
{
   return (money_t)data->value/100; 
}

money_t intra_count_stub(is_data_t * data, acc_t * acc)
{
   return 0; // Intranet is manual only
}

int no_of_days[]=
{  31, // jan
   28, // feb (no leap day)
   31, // mar
   30, // apr
   31, // may
   30, // jun
   31, // jul
   31, // aug
   30, // sep
   31, // oct
   30, // nov
   31  // dec
};

money_t charge_count_proc(is_data_t * data, acc_t * acc)
{  money_t    val = 0;
   int        tariff     = 0;  // set to global default
   int        i;
   time_t     curtime = time(NULL);
   struct tm  stm;
   int        days;

// count number of days in current month
   localtime_r(&curtime, &stm);      
   days = no_of_days[stm.tm_mon];

   // february leap day test 
   stm.tm_mday = 29;
   if (stm.tm_mon == 1 && timelocal(&stm) > 0) days++;

// Find tariff index by tariff
// (last matching wins)
   for (i=1; charge_tariffs[i].tariff >= 0; i++)
   {  if (acc->tariff == charge_tariffs[i].tariff) tariff = i;
   }

// get price value
   val = charge_tariffs[tariff].price;

// count transaction sum
   val /= days;

   return -val;
}

money_t inet_charge_proc(acc_t * acc)
{  money_t    val    = 0;
   int        tariff = 0;  // set to global default
   int        i;
   time_t     curtime = time(NULL);
   struct tm  stm;
   int        days;

// count number of days in current month
   localtime_r(&curtime, &stm);      
   days = no_of_days[stm.tm_mon];

   // february leap day test 
   stm.tm_mday = 29;
   if (stm.tm_mon == 1 && timelocal(&stm) > 0) days++;

// Find tariff index by tariff
// (last matching wins)
   for (i=1; inet_tariffs[i].tariff >= 0; i++)
   {  if (acc->tariff == inet_tariffs[i].tariff) tariff = i;
   }

// get price value
   val = inet_tariffs[tariff].month_charge;

// count transaction sum
   val /= days;

   return -val;
}

