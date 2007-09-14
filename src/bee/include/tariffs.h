#ifndef __TARIFFS_H__
#define __TARIFFS_H__

typedef struct
{  int      tariff;
   money_t  min;
} limit_t;

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
} inet_tariff_t;

typedef struct
{  int      tariff;   // tariff number
   money_t  price;    // month fee
} charge_tariff_t;


limit_t         * limits;

inet_tariff_t   * inet_tariffs;
charge_tariff_t * charge_tariffs;

#endif /* __TARIFFS_H__*/

