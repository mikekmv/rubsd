/* $RuOBSD: res.c,v 1.10 2006/09/29 04:35:25 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <db.h>
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
{  int      reserv;
   int      weekday;
   int      hour_from;
   int      hour_to;
   money_t  price_in;
   money_t  price_out;
} inet_tariff_t;

inet_tariff_t  inet_tariffs[] =
{
   { 0, (-1),  0,  0,  2.2, 2.2},  // default price (global default)
   { 0, (-1),  2,  4,  1.5, 1.5},  // night dead time
   { 0, (-1),  4,  9,  0.5, 0.5},  // night dead time
   { 0, (-1),  9, 19,  2.3, 2.3},  // day rush hour
   { 0,   0,   9, 19, (-1),(-1)},  // day rush hour (sunday)
   { 0,   6,   9, 19, (-1),(-1)},  // day rush hour (saturday)

   {-1,  -1,  -1, -1,   -1, -1 }   // (terminator)
};

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
   localtime_r(&curtime, &stm);

// Find default tariff (for reserved account value)
// (last matching wins)
   for (i=0; inet_tariffs[i].hour_from >= 0; i++)
   {  if (acc->reserv[0] == inet_tariffs[i].reserv &&
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

// Find tariff index by time & reserv & weekday
// (last matching wins)
   for (i=1; inet_tariffs[i].hour_from >= 0; i++)
   {  if (acc->reserv[0] == inet_tariffs[i].reserv              &&
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

