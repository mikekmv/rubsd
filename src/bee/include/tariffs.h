#ifndef __TARIFFS_H__
#define __TARIFFS_H__

typedef struct
{  int    tariff;
   char * name;
} tariff_info_t;

typedef struct
{  int         tariff;
   double      min;
   long long   res_min;
} tariff_limit_t;

#define INET_TFLAG_SIN   0x00000001
#define INET_TFLAG_SOUT  0x00000002

typedef struct
{  int        tariff;
   int        weekday;
   int        hour_from;
   int        hour_to;
   double     price_in;
   double     price_out;
   double     month_charge;
   int        sw_tariff;
   double     sw_summ;
   long long  sw_inetsumm;
   u_int      flags;
} tariff_inet_t;

extern tariff_info_t   * tariffs_info;
extern tariff_limit_t  * tariffs_limit;
extern tariff_inet_t   * tariffs_inet;

__BEGIN_DECLS

int tariffs_load(char * filename);
int tariffs_free();

__END_DECLS

#endif /* __TARIFFS_H__*/
