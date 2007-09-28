#ifndef __BEECONF_H__
#define __BEECONF_H__

extern char  * conf_coreaddr;
extern int     conf_coreport;
extern char  * conf_workdir;
extern char  * conf_dbdir;
extern char  * conf_accfile;
extern char  * conf_logfile;
extern char  * conf_gatefile;
extern char  * conf_tariffile;
extern char  * conf_resfile;
extern char  * conf_logindex;
extern char  * conf_gatetemp;
extern char  * conf_gatelock;
extern char  * conf_updlock;
extern char  * conf_grantmask;
extern char  * conf_denymask;
extern char  * conf_applyscript;
extern char  * conf_intrascript;
extern char  * conf_newloginscript;
extern char  * conf_paymanframes;
extern char  * conf_paymanusers;
extern char  * conf_paymanlog;

extern char  * default_conf;  // default config name (Read-only)

__BEGIN_DECLS

int conf_load(char * file);
int conf_free();

__END_DECLS

#endif /*__BEECONF_H__*/

