/* $RuOBSD: res.c,v 1.1 2005/07/30 22:43:13 shadow Exp $ */

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
   {0, NULL,  "inet",   NULL, 1},
   {0, NULL,  "mail",   NULL, 0},
   {0, NULL, "adder",   NULL, 0},
   {0, NULL, "intra",   NULL, 0},
   {0, NULL,"charge",   NULL, 0}
};

