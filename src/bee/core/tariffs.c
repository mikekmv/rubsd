#include <bee.h>
#include <tariffs.h>

limit_t    limits_a[]=
{
  { 0,     0.00 }, // default limit
  { 1,   -50.00 },
  { 2, -2000.00 },

  {-1, -1       }  // (terminator)
};

inet_tariff_t  inet_tariffs_a[] =
{
   { 0, (-1),  0,  0,  1.8, 1.8},  // default price (global default)
   { 0, (-1),  0,  2,  1.5, 1.5},  // night dead time
   { 0, (-1),  2,  4,  1.3, 1.3},  // night dead time
   { 0, (-1),  4,  9,  0.5, 0.5},  // night dead time
   { 0, (-1),  9, 19,  2.0, 2.0},  // day rush hour
   { 0,   0,   9, 19, (-1),(-1)},  // day rush hour (sunday)
   { 0,   6,   9, 19, (-1),(-1)},  // day rush hour (saturday)

// same prices, but 50 rub credit
   { 1, (-1),  0,  0,  1.8, 1.8},  // default price
   { 1, (-1),  0,  2,  1.5, 1.5},  // night dead time
   { 1, (-1),  2,  4,  1.3, 1.3},  // night dead time
   { 1, (-1),  4,  9,  0.5, 0.5},  // night dead time
   { 1, (-1),  9, 19,  2.0, 2.0},  // day rush hour
   { 1,   0,   9, 19, (-1),(-1)},  // day rush hour (sunday)
   { 1,   6,   9, 19, (-1),(-1)},  // day rush hour (saturday)

// same prices, but 2000 rub credit
   { 2, (-1),  0,  0,  1.8, 1.8},  // default price
   { 2, (-1),  0,  2,  1.5, 1.5},  // night dead time
   { 2, (-1),  2,  4,  1.3, 1.3},  // night dead time
   { 2, (-1),  4,  9,  0.5, 0.5},  // night dead time
   { 2, (-1),  9, 19,  2.0, 2.0},  // day rush hour
   { 2,   0,   9, 19, (-1),(-1)},  // day rush hour (sunday)
   { 2,   6,   9, 19, (-1),(-1)},  // day rush hour (saturday)

// origin, for unlim64
   { 3, (-1),  0,  0,    0,   0},  // default price

// origin, for unlim128
   { 4, (-1),  0,  0,    0,   0},  // default price

   {-1,  -1,  -1, -1,   -1, -1 }          // (terminator)
};

charge_tariff_t  charge_tariffs_a[] =
{
  {0, 0},       // (global default)
  {1, 0},
  {2, 0},
  {3, 1990},
  {4, 3290},

  {(-1), (-1)}  // terminator
};

limit_t         * limits         = limits_a;
inet_tariff_t   * inet_tariffs   = inet_tariffs_a;
charge_tariff_t * charge_tariffs = charge_tariffs_a;

