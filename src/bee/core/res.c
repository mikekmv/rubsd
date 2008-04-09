/* $RuOBSD: res.c,v 1.24 2008/02/08 04:04:07 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <db.h>
#include <res.h>
#include <tariffs.h>

void res_coreinit()
{
   resource[RES_INET].count     = inet_count_proc;
   resource[RES_MAIL].count     = mail_count_proc;
   resource[RES_ADDER].count    = adder_count_proc;
   resource[RES_INTRA].count    = intra_count_stub;

   resource[RES_INET].charge    = inet_charge_proc;
}

#define DELIM  " ,\t\n\r"

double inet_count_proc(is_data_t * data, acc_t * acc)
{  double     val = 0;
   time_t     curtime;
   time_t     stime;
   time_t     etime;
   struct tm  stm;
   int        def_tariff    = 0;  // (tariff index) set to global default
   int        tariff        = 0;  // (tariff index) set to global default

   int        target_plan   = 0;  // effective tariff plan number
   long long  inet_start    = 0;
   double     money_start   = 0;
   long long  acc_inet_summ = 0;  // account month traffic summary temp storage
   int        i;
   int        loop = 0;         // tariffs loop detection counter

#define CORR_VALUE           600      /* 10 minutes */

// Get current time
   curtime = time(NULL);

// Summ traffic & money
// check reset time (& reset if reached)
   if (curtime >= acc->summ_rsttime)
   {

   // reset counters, set new reset time
      acc->inet_summ_in  = 0;
      acc->inet_summ_out = 0;
      acc->money_summ    = 0;

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

// Break-down current time to values
   localtime_r(&curtime, &stm);

   target_plan   = acc->tariff;
   inet_start    = 0;
   money_start   = 0;

   fprintf(stderr, "DEBUG: initial tariff %d\n", target_plan);

   while(1) // hack - conditional cycle
   {
// Tariffs loop detection
      if ((loop++) > 256)
      {  syslog(LOG_ERR, "TARIFF LOOP detected on plan %d (inet = %lld/%lld money = %g)", 
                acc->tariff, acc->inet_summ_in, acc->inet_summ_out, acc->money_summ);

         fprintf(stderr, "DEBUG: tarif loop detected: abort\n");
         return 0;
      } 

// Find default tariff (for account tariff number)
// (last matching wins)
      for (i=0; tariffs_inet[i].hour_from >= 0; i++)
      {  if (target_plan == tariffs_inet[i].tariff &&
             (tariffs_inet[i].weekday < 0 || tariffs_inet[i].weekday == stm.tm_wday) &&
             tariffs_inet[i].hour_from == 0           &&
             tariffs_inet[i].hour_to   == 0) def_tariff = i;
      }

// Check boundaries & switch target tariff plan if reached
      if (tariffs_inet[def_tariff].sw_tariff >= 0)
      {  acc_inet_summ = 0;
         if ((tariffs_inet[def_tariff].flags & INET_TFLAG_SIN)  == 0) acc_inet_summ += acc->inet_summ_in;
         if ((tariffs_inet[def_tariff].flags & INET_TFLAG_SOUT) == 0) acc_inet_summ += acc->inet_summ_out;
         
         if (tariffs_inet[def_tariff].sw_inetsumm != 0 &&          // switch on inet
             tariffs_inet[def_tariff].sw_inetsumm <= (acc_inet_summ - inet_start))
         {  target_plan  = tariffs_inet[def_tariff].sw_tariff;
            inet_start  += tariffs_inet[def_tariff].sw_inetsumm;
            fprintf(stderr, "DEBUG: switching to tariff %d (inet)\n", tariffs_inet[def_tariff].sw_tariff);
            continue;
         }
         else
         {  if (tariffs_inet[def_tariff].sw_summ != 0 &&           // switch on money
               tariffs_inet[def_tariff].sw_summ  <= (acc->money_summ - money_start)) 
            {  target_plan  = tariffs_inet[def_tariff].sw_tariff;
               money_start += tariffs_inet[def_tariff].sw_summ;
               fprintf(stderr, "DEBUG: switching to tariff %d (money)\n", tariffs_inet[def_tariff].sw_tariff);
               continue;
            }
            else
            {  if (tariffs_inet[def_tariff].sw_summ     == 0 &&    // tariff plan alias
                   tariffs_inet[def_tariff].sw_inetsumm == 0)
               {  target_plan = tariffs_inet[def_tariff].sw_tariff;
                  fprintf(stderr, "DEBUG: switching to tariff %d (alias)\n", tariffs_inet[def_tariff].sw_tariff);
                  continue;
               }
            }
         }
      }
// Exit cycle after tariff plan has been estabilished
      break;  
   } // while (1);                         

   fprintf(stderr, "DEBUG: result tariff %d (def index %d)\n", target_plan, def_tariff);

// Set tariff index to default
   tariff = def_tariff;

// Count day start time (0:00)
   stm.tm_hour = 0;
   stm.tm_min  = 0;
   stm.tm_sec  = 0;   

// Add realtime time correction
   stime += CORR_VALUE;  

// Find tariff index by time & tariff & weekday
// (last matching wins)
   for (i=1; tariffs_inet[i].hour_from >= 0; i++)
   {
      stm.tm_hour = tariffs_inet[i].hour_from; 
      stime  = timelocal(&stm);

      stm.tm_hour = tariffs_inet[i].hour_to; 
      etime  = timelocal(&stm);

      if (target_plan == tariffs_inet[i].tariff                 &&
          (tariffs_inet[i].weekday < 0 || tariffs_inet[i].weekday == stm.tm_wday) &&
          tariffs_inet[i].hour_from != tariffs_inet[i].hour_to  &&
          curtime >= stime &&  curtime <  etime )  tariff = i;
   }

// trap default tariff field
   if (tariffs_inet[tariff].price_in < 0) tariff = def_tariff;

   fprintf(stderr, "DEBUG: result index %d\n", tariff);

// get price value
   if ((data->proto_id & 0x80000000) == 0) 
      val = tariffs_inet[tariff].price_in;
   else 
      val = tariffs_inet[tariff].price_out;

// count transaction sum
   val = (double)data->value * val / (1024*1024);

// Add summary values to account
   acc->money_summ += val;

   if ((data->proto_id & 0x80000000) == 0) 
      acc->inet_summ_in  += data->value;
   else 
      acc->inet_summ_out += data->value;

// Store tariff plan & tariff index for logging
   data->proto2 |= (target_plan << 16) | ((tariff - def_tariff) << 24);

   return -val;
}

double mail_count_proc(is_data_t * data, acc_t * acc)
{
   return 0; // Mail is for free ;)
}

double intra_count_stub(is_data_t * data, acc_t * acc)
{
   return 0; // Intranet is manual only
}

double adder_count_proc(is_data_t * data, acc_t * acc)
{
   return (double)data->value/100;
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

double inet_charge_proc(acc_t * acc)
{  double     val    = 0;
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
   for (i=1; tariffs_inet[i].tariff >= 0; i++)
   {  if (acc->tariff == tariffs_inet[i].tariff) tariff = i;
   }

// get price value
   val = tariffs_inet[tariff].month_charge;

// count transaction sum
   val /= days;

   acc->money_summ += val;

   return -val;
}

