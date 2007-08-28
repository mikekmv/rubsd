/* $RuOBSD: core.h,v 1.9 2007/08/23 08:43:46 shadow Exp $ */
 
#ifndef __CORE_H__
#define __CORE_H__

#define OPTS "hA:udco"

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
int acc_charge_trans (accbase_t * base, logbase_t * logbase, int accno, is_data_t * isdata);

int     accs_state(acc_t * acc);
money_t acc_limit (acc_t * acc);

#endif /* __CORE_H__ */
