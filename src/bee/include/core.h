/* $RuOBSD: core.h,v 1.6 2005/07/02 23:45:47 shadow Exp $ */
 
#ifndef __CORE_H__
#define __CORE_H__

#define OPTS "hA:udc"

extern char * __progname;

extern logbase_t   Logbase;
extern accbase_t   Accbase;
extern link_t    * ld;
extern int         NeedUpdate;
extern int         HumanRead;
extern int         MachineRead;
extern char      * linkfile_name;
extern char      * IntraScript;

void usage(int code);
int  access_update();

int acc_transaction (accbase_t * base, logbase_t * logbase, int accno, is_data_t * isdata, int arg);

int accs_state(acc_t * acc);

#endif /* __CORE_H__ */
