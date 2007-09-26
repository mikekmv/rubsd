#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <syslog.h>

#include <bee.h>

char  * conf_coreaddr;
int     conf_coreport;
char  * conf_workdir;
char  * conf_dbdir;
char  * conf_accfile;
char  * conf_logfile;
char  * conf_gatefile;
char  * conf_tariffile;
char  * conf_resfile;
char  * conf_logindex;
char  * conf_gatetemp;
char  * conf_gatelock;
char  * conf_updlock;
char  * conf_grantmask;
char  * conf_denymask;
char  * conf_applyscript;
char  * conf_intrascript;
char  * conf_newloginscript;
char  * conf_paymanframes;
char  * conf_paymanusers;
char  * conf_paymanlog;

static g3c_ruleparam ipc_params[]=
{  {"core_addr", PT_STRING,   1, NULL, 0,        20, NULL, NULL},
   {"core_port", PT_INTEGER,  1, NULL, 1,     65535, NULL, NULL}
};

static g3c_ruleparam dirs_params[]=
{  {"workdir", PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"dbdir",   PT_STRING,   1, NULL, 0,      1024, NULL, NULL}
};

static g3c_ruleparam files_params[]=
{  {"accfile",        PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"logfile",        PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"gatefile",       PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"tariffile",      PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"resfile",        PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"logindex",       PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"gatetemp",       PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"gatelock",       PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"updlock",        PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"grantfile_mask", PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"denyfile_mask",  PT_STRING,   1, NULL, 0,      1024, NULL, NULL}
};

static g3c_ruleparam core_params[]=
{  {"apply_script",    PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"intra_script",    PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"newlogin_script", PT_STRING,   1, NULL, 0,      1024, NULL, NULL}
};

static g3c_ruleparam payman_params[]=
{  {"framefile", PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"usersfile", PT_STRING,   1, NULL, 0,      1024, NULL, NULL},
   {"logfile",   PT_STRING,   1, NULL, 0,      1024, NULL, NULL}
};

static g3c_rulesec main_defs[]=
{  {0,  2, "ipc",     NULL, ipc_params,    0, 1},
   {0,  2, "dirs",    NULL, dirs_params,   0, 1},
   {0, 11, "files",   NULL, files_params,  0, 1},
   {0,  3, "core",    NULL, core_params,   0, 1},
   {0,  3, "payman",  NULL, payman_params, 0, 1},
   {0,0,NULL,NULL,NULL,0,0} /* terminator */
};

static g3c_rulesec g3c_rules[]=
{  {2,  0, "main", main_defs, NULL,          0, 0},
   {0,  2, "ipc", NULL,       ipc_params,    0, 0},
   {0,  2, "dirs", NULL,      dirs_params,   0, 0},
   {0, 11, "files", NULL,     files_params,  0, 0},
   {0,  3, "core", NULL,      core_params,   0, 0},
   {0,  3, "payman", NULL,    payman_params, 0, 0},
   {0,0,NULL,NULL,NULL,0,0} /* terminator */
};


static g3c_file       fs;
static g3c_section  * pss = NULL;
static g3c_pos        pos;

static char         default_conf[] = "/etc/bee/bee.conf";

int conf_load(char * file)
{  int       rc;

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
      pss = NULL;
      return (-1);
   }

// init search structure
   rc = g3c_reset(&pos, pss);
   if (rc < 0)
   {  g3c_free(pss);
      pss = NULL;
      return (-1);
   }

// Load ipc section
   rc = g3c_nextn(&pos, "ipc");
   if (rc < 0)
   {  syslog(LOG_ERR, "conf_load(): FATAL - No 'ipc' section found");
      g3c_free(pss);
      pss = NULL;
      return (-1);
   }
   conf_coreaddr = (char*)g3c_allocvalue(&pos, "core_addr", PT_STRING);
   conf_coreport = *g3c_integer(&pos, "core_port");
   g3c_uplevel(&pos);
   
// Load dirs section
   rc = g3c_nextn(&pos, "dirs");
   if (rc < 0)
   {  syslog(LOG_ERR, "conf_load(): FATAL - No 'dirs' section found");
      g3c_free(pss);
      pss = NULL;
      return (-1);
   }
   conf_workdir = (char*)g3c_allocvalue(&pos, "workdir", PT_STRING);
   conf_dbdir   = (char*)g3c_allocvalue(&pos, "dbdir", PT_STRING);
   g3c_uplevel(&pos);

// Load files section
   rc = g3c_nextn(&pos, "files");
   if (rc < 0)
   {  syslog(LOG_ERR, "conf_load(): FATAL - No 'files' section found");
      g3c_free(pss);
      pss = NULL;
      return (-1);
   }
   conf_accfile   = (char*)g3c_allocvalue(&pos, "accfile", PT_STRING);
   conf_logfile   = (char*)g3c_allocvalue(&pos, "logfile", PT_STRING);
   conf_gatefile  = (char*)g3c_allocvalue(&pos, "gatefile", PT_STRING);
   conf_tariffile = (char*)g3c_allocvalue(&pos, "tariffile", PT_STRING);
   conf_resfile   = (char*)g3c_allocvalue(&pos, "resfile", PT_STRING);
   conf_logindex  = (char*)g3c_allocvalue(&pos, "logindex", PT_STRING);
   conf_gatetemp  = (char*)g3c_allocvalue(&pos, "gatetemp", PT_STRING);
   conf_gatelock  = (char*)g3c_allocvalue(&pos, "gatelock", PT_STRING);
   conf_updlock   = (char*)g3c_allocvalue(&pos, "updlock", PT_STRING);
   conf_grantmask = (char*)g3c_allocvalue(&pos, "grantfile_mask", PT_STRING);
   conf_denymask  = (char*)g3c_allocvalue(&pos, "denyfile_mask", PT_STRING);
   g3c_uplevel(&pos);

// Load core section
   rc = g3c_nextn(&pos, "core");
   if (rc < 0)
   {  syslog(LOG_ERR, "conf_load(): FATAL - No 'core' section found");
      g3c_free(pss);
      pss = NULL;
      return (-1);
   }
   conf_applyscript    = (char*)g3c_allocvalue(&pos, "apply_script", PT_STRING);
   conf_intrascript    = (char*)g3c_allocvalue(&pos, "intra_script", PT_STRING);
   conf_newloginscript = (char*)g3c_allocvalue(&pos, "newlogin_script", PT_STRING);
   g3c_uplevel(&pos);

// Load payman section
   rc = g3c_nextn(&pos, "payman");
   if (rc < 0)
   {  syslog(LOG_ERR, "conf_load(): FATAL - No 'payman' section found");
      g3c_free(pss);
      pss = NULL;
      return (-1);
   }
   conf_paymanframes = (char*)g3c_allocvalue(&pos, "framefile", PT_STRING);
   conf_paymanusers  = (char*)g3c_allocvalue(&pos, "usersfile", PT_STRING);
   conf_paymanlog    = (char*)g3c_allocvalue(&pos, "logfile",   PT_STRING);
   g3c_uplevel(&pos);

// free configuration image
   g3c_free(pss);
   pss = NULL;

   return 0;
}


int conf_free()
{
   FREE(conf_coreaddr);
   FREE(conf_workdir);
   FREE(conf_dbdir);
   FREE(conf_accfile);
   FREE(conf_logfile);
   FREE(conf_gatefile);
   FREE(conf_tariffile);
   FREE(conf_resfile);
   FREE(conf_logindex);
   FREE(conf_gatetemp);
   FREE(conf_gatelock);
   FREE(conf_grantmask);
   FREE(conf_denymask);
   FREE(conf_applyscript);
   FREE(conf_intrascript);
   FREE(conf_newloginscript);
   FREE(conf_paymanframes);
   FREE(conf_paymanusers);
   FREE(conf_paymanlog);

   return 0;
}
