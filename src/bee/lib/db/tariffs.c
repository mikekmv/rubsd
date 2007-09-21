#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include <bee.h>
#include <g3c.h>
#include <tariffs.h>

tariff_info_t   * tariffs_info   = NULL;
tariff_limit_t  * tariffs_limit  = NULL;
tariff_inet_t   * tariffs_inet   = NULL;

int  tariffs_info_cnt   = 0;
int  tariffs_limit_cnt  = 0;
int  tariffs_inet_cnt   = 0;

g3c_ruleparam inet_params[]=
{  {"weekday",   PT_INTEGER,  0, NULL, 0,         6, NULL, NULL},
   {"hour_from", PT_INTEGER,  0, NULL, 0,        23, NULL, NULL},
   {"hour_to",   PT_INTEGER,  0, NULL, 0,        23, NULL, NULL},
   {"price",     PT_STRING,   0, NULL, 0,        10, NULL, NULL},
   {"inbound",   PT_STRING,   0, NULL, 0,        10, NULL, NULL},
   {"outbound",  PT_STRING,   0, NULL, 0,        10, NULL, NULL},
   {"charge",    PT_STRING,   0, NULL, 0,        10, NULL, NULL},
   {"sw_to",     PT_INTEGER,  0, NULL, 0,   INT_MAX, NULL, NULL},
   {"sw_bytes",  PT_LONGLONG, 0, NULL, 0, LLONG_MAX, NULL, NULL},
   {"sw_money",  PT_STRING,   0, NULL, 0,        40, NULL, NULL},
   {"sw_on",     PT_STRING,   0, NULL, 0,         4, NULL, NULL}
};

g3c_ruleparam plan_params[]=
{  {"number", PT_INTEGER, 1, NULL, 0, INT_MAX, NULL, NULL},
   {"name",   PT_STRING,  1, NULL, 0,      40, NULL, NULL},
   {"credit", PT_STRING,  0, NULL, 0,      10, NULL, NULL}
};

g3c_rulesec main_defs[]=
{  {0, 3, "plan",     NULL, plan_params, 0, 1},
   {0,0,NULL,NULL,NULL,0,0} /* terminator */
};

g3c_rulesec g3c_rules[]=
{  {2,  0, "main", main_defs, NULL,        0, 0},
   {0,  3, "plan", NULL,      plan_params, 0, 0},
   {0, 11, "inet", NULL,      inet_params, 0, 0},
   {0,0,NULL,NULL,NULL,0,0} /* terminator */
};


g3c_file       fs;
g3c_section  * pss = NULL;
g3c_pos        pos;

int param_dump(g3c_param *par)
{
   fprintf(stderr, "  name='%s' value=", par->name);
   if (par->type == PT_STRING)   fprintf(stderr, "'%s'", (char*)par->value);
   if (par->type == PT_INTEGER)  fprintf(stderr, "%d", *((int*)par->value));
   if (par->type == PT_LONGLONG) fprintf(stderr, "%lld", *((long long*)par->value));

   fprintf(stderr, " type=%d, line=%d\n", par->type, par->line);
   return 0;  
}

int section_dump(g3c_section  *sec)
{  int i;

   fprintf(stderr, "sc=%d, pc=%d, par=%p, subsec=%p, line=%d\n",
           sec->sc, sec->pc, sec->params, sec->sections, sec->line);

   for (i=0; i < sec->pc; i++)
   {  param_dump(sec->params + i);
   }   

   for (i=0; i < sec->sc; i++)
   {  section_dump(sec->sections + i);
   }

   return 0;
}


int tariffs_load(char * file)
{  int       rc;
   void    * val;
   int       first;

   tariff_info_t  ti;
   tariff_limit_t tl;
   tariff_inet_t  tn;

   if (file == NULL) return (-1);

// init file structure
   rc = g3c_file_init(&fs, file);
   if (rc != G3C_SUCCESS) return (-1);

// load configuration file
   rc = g3c_file_load(&fs, NULL);
   if (rc != G3C_SUCCESS) 
   {  g3c_file_free(&fs);
      return (-1);
   }


// parse file to structure
   rc = g3c_parse(&fs, &pss, g3c_rules);
   g3c_file_free(&fs);
   if (rc != G3C_SUCCESS)
   {  g3c_free(pss);
      return (-1);
   }

//   section_dump(pss);

// init search structure
   rc = g3c_reset(&pos, pss);
   if (rc < 0)
   {  g3c_free(pss);
      return (-1);
   }

   while(1)
   {
   // find plan section
      rc = g3c_nextn(&pos, "plan");
      if (rc < 0) break;
   // load tariff plan values
      ti.tariff = *g3c_integer(&pos, "number");
      ti.name   = (char*)g3c_allocvalue(&pos, "name", PT_STRING);

//fprintf(stderr, "tariff %d, name = '%s'\n", ti.tariff, ti.name); 

   // insert info
      da_ins(&tariffs_info_cnt, &tariffs_info, sizeof(tariff_info_t), (-1), &ti);
   // load credit limit
      val = (void*)g3c_string(&pos, "credit");
      if (val != NULL || ti.tariff == 0)
      {  tl.tariff = ti.tariff;
         if (val != NULL) tl.min = - strtod((char *)val, NULL);
         else tl.min = 0;

//fprintf(stderr, "credit %g\n", tl.min);

         da_ins(&tariffs_limit_cnt, &tariffs_limit, sizeof(tariff_limit_t), (-1), &tl);
      }
      first = 1;
      while(1)
      {
      // find inet section
         rc = g3c_nextn(&pos, "inet");
         if (rc < 0) break;
      // initialize w/ defaults
         tn.tariff       = ti.tariff;
         tn.weekday      = (-1);
         tn.hour_from    = 0;
         tn.hour_to      = 0;
         tn.price_in     = first ? 0 : (-1);
         tn.price_out    = first ? 0 : (-1);
         tn.month_charge = 0;
         tn.sw_tariff    = (-1);
         tn.sw_summ      = 0; 
         tn.sw_inetsumm  = 0;
         tn.flags        = 0;
      // load values
         val = (void*)g3c_integer(&pos, "weekday");
         if (val != NULL) tn.weekday = *((int *)val);

//fprintf(stderr, "weekday=%p\n", val);
               
         val = (void*)g3c_integer(&pos, "hour_from");
         if (val != NULL) tn.hour_from = *((int *)val);

//fprintf(stderr, "hour_from=%p\n", val);

         val = (void*)g3c_integer(&pos, "hour_to");
         if (val != NULL) tn.hour_to = *((int *)val);
           
//fprintf(stderr, "hour_to=%p\n", val);

         val = g3c_getvalue(&pos, "price", PT_STRING);
         if (val != NULL)
         {  tn.price_in  = strtod((char *)val, NULL);
            tn.price_out = tn.price_in;
         }

//fprintf(stderr, "price=%p\n", val);
         
         val = g3c_getvalue(&pos, "inbound", PT_STRING);
         if (val != NULL) tn.price_in = strtod((char *)val, NULL);

//fprintf(stderr, "inbound=%p\n", val);

         val = g3c_getvalue(&pos, "outbound", PT_STRING);
         if (val != NULL) tn.price_out = strtod((char *)val, NULL);

//fprintf(stderr, "outbound=%p\n", val);

         val = g3c_getvalue(&pos, "charge", PT_STRING);
         if (val != NULL) tn.month_charge = strtod((char *)val, NULL);

//fprintf(stderr, "charge=%p\n", val);

         val = (void*)g3c_integer(&pos, "sw_to");
         if (val != NULL) tn.sw_tariff = *((int *)val);

//fprintf(stderr, "sw_to =%p\n", val);

         val = g3c_getvalue(&pos, "sw_money", PT_STRING);
         if (val != NULL) tn.sw_summ = strtod((char *)val, NULL);
                    
//fprintf(stderr, "sw_money=%p\n", val);

         val = (void*)g3c_longlong(&pos, "sw_bytes");
         if (val != 0) tn.sw_inetsumm = *((long long *)val);

//fprintf(stderr, "sw_bytes=%p\n", val);

         val = g3c_getvalue(&pos, "sw_on", PT_STRING);
         if (val != NULL)
         {  if (strcasecmp((char*)val, "in") == 0)  tn.flags = INET_TFLAG_SOUT;
            if (strcasecmp((char*)val, "out") == 0) tn.flags = INET_TFLAG_SIN;
         }

//fprintf(stderr, "sw_on=%p\n", val);

         da_ins(&tariffs_inet_cnt, &tariffs_inet, sizeof(tariff_inet_t), (-1), &tn);

         first = 0;
         g3c_uplevel(&pos);       
      }
      g3c_uplevel(&pos);       
   }

// free configuration image
   g3c_free(pss);
   pss = NULL;

   if (tariffs_info == NULL || tariffs_limit == NULL || tariffs_inet == NULL)
      return (-1);

// add terminators
   ti.tariff = (-1);
   ti.name   = NULL;
   da_ins(&tariffs_info_cnt, &tariffs_info, sizeof(tariff_info_t), (-1), &ti);
   tl.tariff = (-1);
   tl.min    = (-1);
   da_ins(&tariffs_limit_cnt, &tariffs_limit, sizeof(tariff_limit_t), (-1), &tl);
   tn.tariff       = (-1);
   tn.weekday      = (-1);
   tn.hour_from    = (-1);
   tn.hour_to      = (-1);
   tn.price_in     = (-1);
   tn.price_out    = (-1);
   tn.month_charge = (-1);
   tn.sw_tariff    = (-1);
   tn.sw_summ      = (-1); 
   tn.sw_inetsumm  = (-1);
   tn.flags        = 0;
   da_ins(&tariffs_inet_cnt, &tariffs_inet, sizeof(tariff_inet_t), (-1), &tn);         
   
   return 0;
}


int tariffs_free()
{  int i;

   for(i=0; i<tariffs_info_cnt; i++)
   {  free(tariffs_info[i].name);
      tariffs_info[i].name = NULL;
   } 

   da_empty(&tariffs_info_cnt,  &tariffs_info,   sizeof(tariff_info_t));
   da_empty(&tariffs_limit_cnt, &tariffs_limit,  sizeof(tariff_limit_t));
   da_empty(&tariffs_inet_cnt,  &tariffs_info,   sizeof(tariff_inet_t));

   return 0;
}





