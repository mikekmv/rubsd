/* $RuOBSD: res.c,v 1.6 2003/07/28 03:19:46 shadow Exp $ */

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

money_t inet_count_proc(is_data_t * data)
{
#define INET_1M_PRICE 3

// (Uncomment for free Outbound traffic)
//   if ((data->proto_id & 0x80000000) != 0) return 0; 

// Inbound traffic
     return -((money_t)data->value * INET_1M_PRICE /(1024*1024));   
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

