/* $RuOBSD: res.c,v 1.2 2007/08/23 07:40:32 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <res.h>

int         resourcecnt=5;
resource_t  resource[]=
{  
   {0, NULL, NULL,  "inet",   NULL, 1},
   {0, NULL, NULL,  "mail",   NULL, 0},
   {0, NULL, NULL, "adder",   NULL, 0},
   {0, NULL, NULL, "intra",   NULL, 0},
   {0, NULL, NULL,"charge",   NULL, 0}
};

