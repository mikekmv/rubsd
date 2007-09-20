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
   { 0, (-1),  0,  0,  1.8, 1.8, 0, (-1), 0, 0, 0},  // default price (global default)
   { 0, (-1),  0,  2,  1.5, 1.5, 0, (-1), 0, 0, 0},  // night dead time
   { 0, (-1),  2,  4,  1.3, 1.3, 0, (-1), 0, 0, 0},  // night dead time
   { 0, (-1),  4,  9,  0.5, 0.5, 0, (-1), 0, 0, 0},  // night dead time
   { 0, (-1),  9, 19,  2.0, 2.0, 0, (-1), 0, 0, 0},  // day rush hour
   { 0,   0,   9, 19, (-1),(-1), 0, (-1), 0, 0, 0},  // day rush hour (sunday)
   { 0,   6,   9, 19, (-1),(-1), 0, (-1), 0, 0, 0},  // day rush hour (saturday)

// same prices, but 50 rub credit
   { 1, (-1),  0,  0,    0,   0, 0,    0, 0, 0, 0},  // ALIAS to plan 0
// same prices, but 2000 rub credit
   { 2, (-1),  0,  0,    0,   0, 0,    0, 0, 0, 0},  // ALIAS to plan 0

// unlim-64
   { 3, (-1),  0,  0,    0,   0, 1990, (-1), 0, 0, 0}, 
// unlim-128
   { 4, (-1),  0,  0,    0,   0, 3290, (-1), 0, 0, 0},

   { 5, (-1),  0,  0,    0,   0, 500,  0, 0, 104857600, 0},  // test 

   {(-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), 0}          // (terminator)
};

limit_t         * limits         = limits_a;
inet_tariff_t   * inet_tariffs   = inet_tariffs_a;


