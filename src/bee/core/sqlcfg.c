#include <sys/types.h>
#include <stdlib.h>
#include <g3c.h>
#include "sqlcfg.h"

#include <db.h>
#include <conf.h>

// G3C Rules

g3c_ruleparam sql_rules[]=
{  {"sqltype",  PT_STRING, 0, "PGSQL",     0, 8,  NULL, NULL},
   {"hostname", PT_STRING, 0, "127.0.0.1", 0, 64, NULL, NULL},
   {"login",    PT_STRING, 0, "bee",       0, 16, NULL, NULL},
   {"password", PT_STRING, 1, NULL,        0, 32, NULL, NULL},
   {"dbname",   PT_STRING, 0, "beedb",     0, 16, NULL, NULL},
   {"scripts",  PT_STRING, 0, "/usr/local/libdata/bee/scripts.dat", 0, 256, NULL, NULL},
};

g3c_rulesec main_defs[]=
{  {0, 1, "sql",  NULL, sql_rules, 0, 1},
   {0,0,NULL,NULL,NULL,0,0} /* terminator */
};

g3c_rulesec g3c_rules[]=
{  {1, 0, "main", main_defs, NULL,      0, 0},
   {0, 6, "sql",  NULL,      sql_rules, 0, 0},
   {0,0,NULL,NULL,NULL,0,0} /* terminator */
};

// Load configuration:
// 1. Load file
// 2. Make image
// 3. Collect configuration values

g3c_file       fs;
g3c_section  * pss;
g3c_pos        pos;

int cfg_load(char * file, config_t * pconf)
{
   int        rc;

   if (pconf == NULL) return (-1);

   if (file == NULL) file = default_conf;

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

// init search structure
   rc = g3c_reset(&pos, pss);
   if (rc < 0)
   {  g3c_free(pss);
      return (-1);
   }

// find sql section
   rc = g3c_nextn(&pos, "sql");
   if (rc < 0)
   {  g3c_free(pss);
      return (-1);
   }

// load sql section values   
   pconf->DBtype    = (char*)g3c_allocvalue(&pos, "sqltype",  PT_STRING);
   pconf->DBhost    = (char*)g3c_allocvalue(&pos, "hostname", PT_STRING);
   pconf->DBlogin   = (char*)g3c_allocvalue(&pos, "login",    PT_STRING);
   pconf->DBpass    = (char*)g3c_allocvalue(&pos, "password", PT_STRING);
   pconf->DBname    = (char*)g3c_allocvalue(&pos, "dbname",   PT_STRING);
   pconf->DBscripts = (char*)g3c_allocvalue(&pos, "scripts",  PT_STRING);

// Free configuration structure
   g3c_free(pss);

   return 0;   
}

