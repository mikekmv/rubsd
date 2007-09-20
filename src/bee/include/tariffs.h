#ifndef __TARIFFS_H__
#define __TARIFFS_H__

typedef struct
{  int    tariff;
   char * name;
} tariff_info_t;

typedef struct
{  int      tariff;
   money_t  min;
} tariff_limit_t;

#define INET_TFLAG_SIN   0x00000001
#define INET_TFLAG_SOUT  0x00000002

typedef struct
{  int        tariff;
   int        weekday;
   int        hour_from;
   int        hour_to;
   money_t    price_in;
   money_t    price_out;
   money_t    month_charge;
   int        sw_tariff;
   money_t    sw_summ;
   long long  sw_inetsumm;
   u_int      flags;
} tariff_inet_t;

tariff_info_t   * tariffs_info;
tariff_limit_t  * tariffs_limit;
tariff_inet_t   * tariffs_inet;

int tariffs_load(char * filename);
int tariffs_free();

#endif /* __TARIFFS_H__*/
