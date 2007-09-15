/* $RuOBSD: res.c,v 1.20 2007/09/14 13:53:36 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <db.h>
#include <res.h>
#include <tariffs.h>


/*
int         resourcecnt = 7;
resource_t  resource[]=
{  
   {0, inet_count_proc,    inet_charge_proc,   "inet",	"/usr/local/bin/beepfrules.sh", 1},
   {0, mail_count_proc,                NULL,   "mail",	NULL, 0},
   {0, adder_count_proc,               NULL,  "adder",	NULL, 0},
   {0, intra_count_stub,               NULL,  "intra",	NULL, 0},
   {0, NULL,                           NULL,   "list",	NULL, 0},
   {0, NULL,                           NULL,  "login",	NULL, 0},
   {0, NULL,                           NULL,  "label",	NULL, 0}
};
*/

void res_coreinit()
{
   resource[RES_INET].count     = inet_count_proc;
   resource[RES_MAIL].count     = mail_count_proc;
   resource[RES_ADDER].count    = adder_count_proc;
   resource[RES_INTRA].count    = intra_count_stub;

   resource[RES_INET].charge    = inet_charge_proc;

   resource[RES_INET].ruler_cmd = "/usr/local/bin/beepfrules.sh";
}

#define DELIM  " ,\t\n\r"

money_t inet_count_proc(is_data_t * data, acc_t * acc)
{  money_t    val = 0;
   time_t     curtime;
   time_t     stime;
   struct tm  stm;
   int        def_tariff    = 0;  // (tariff index) set to global default
   int        tariff        = 0;  // (tariff index) set to global default

   int        target_plan   = 0;  // effective tariff plan number
   long long  inet_start    = 0;
   money_t    money_start   = 0;
   long long  acc_inet_summ = 0;  // account month traffic summary temp storage
   int        i;
   int        loop = 0;         // tariffs loop detection counter

#define CORR_VALUE           300      /* 5 minutes */

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
      for (i=0; inet_tariffs[i].hour_from >= 0; i++)
      {  if (target_plan == inet_tariffs[i].tariff &&
             (inet_tariffs[i].weekday < 0 || inet_tariffs[i].weekday == stm.tm_wday) &&
             inet_tariffs[i].hour_from == 0           &&
             inet_tariffs[i].hour_to   == 0) def_tariff = i;
      }

// Check boundaries & switch target tariff plan if reached
      if (inet_tariffs[def_tariff].sw_tariff >= 0)
      {  acc_inet_summ = 0;
         if ((inet_tariffs[def_tariff].flags & INET_TFLAG_SIN)  == 0) acc_inet_summ += acc->inet_summ_in;
         if ((inet_tariffs[def_tariff].flags & INET_TFLAG_SOUT) == 0) acc_inet_summ += acc->inet_summ_out;
         
         if (inet_tariffs[def_tariff].sw_inetsumm != 0 &&          // switch on inet
             inet_tariffs[def_tariff].sw_inetsumm <= (acc_inet_summ - inet_start))
         {  target_plan  = inet_tariffs[def_tariff].sw_tariff;
            inet_start  += inet_tariffs[def_tariff].sw_inetsumm;
            fprintf(stderr, "DEBUG: switching to tariff %d (inet)\n", inet_tariffs[def_tariff].sw_tariff);
            continue;
         }
         else
         {  if (inet_tariffs[def_tariff].sw_summ != 0 &&           // switch on money
               inet_tariffs[def_tariff].sw_summ  <= (acc->money_summ - money_start)) 
            {  target_plan  = inet_tariffs[def_tariff].sw_tariff;
               money_start += inet_tariffs[def_tariff].sw_summ;
               fprintf(stderr, "DEBUG: switching to tariff %d (money)\n", inet_tariffs[def_tariff].sw_tariff);
               continue;
            }
            else
            {  if (inet_tariffs[def_tariff].sw_summ     == 0 &&    // tariff plan alias
                   inet_tariffs[def_tariff].sw_inetsumm == 0)
               {  target_plan = inet_tariffs[def_tariff].sw_tariff;
                  fprintf(stderr, "DEBUG: switching to tariff %d (alias)\n", inet_tariffs[def_tariff].sw_tariff);
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
   {  if (target_plan == inet_tariffs[i].tariff                 &&
          (inet_tariffs[i].weekday < 0 || inet_tariffs[i].weekday == stm.tm_wday) &&
          inet_tariffs[i].hour_from != inet_tariffs[i].hour_to  &&
          curtime >= (stime + inet_tariffs[i].hour_from * 3600) &&
          curtime <  (stime + inet_tariffs[i].hour_to   * 3600))  tariff = i;
   }

// trap default tariff field
   if (inet_tariffs[tariff].price_in < 0) tariff = def_tariff;

   fprintf(stderr, "DEBUG: result index %d\n", tariff);

// get price value
   if ((data->proto_id & 0x80000000) == 0) 
      val = inet_tariffs[tariff].price_in;
   else 
      val = inet_tariffs[tariff].price_out;

// count transaction sum
   val = (money_t)data->value * val / (1024*1024);

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

money_t mail_count_proc(is_data_t * data, acc_t * acc)
{
   return 0; // Mail is for free ;)
}

money_t intra_count_stub(is_data_t * data, acc_t * acc)
{
   return 0; // Intranet is manual only
}

money_t adder_count_proc(is_data_t * data, acc_t * acc)
{
   return (money_t)data->value/100;
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

   acc->money_summ += val;

   return -val;
}

