/* $RuOBSD: core.h,v 1.13 2009/04/09 06:56:27 shadow Exp $ */
 
#ifndef __CORE_H__
#define __CORE_H__

#define OPTS "hudcof:2vt:A:qQ:g"

extern char * __progname;

// DB runtime context
void    * DBdata;

extern logbase_t   Logbase;
extern accbase_t   Accbase;
extern link_t    * ld;
extern int         NeedUpdate;
extern int         HumanRead;
extern int         MachineRead;
extern int         QuietMode;
extern int         ResOn;
extern char        SessionLogin[32];
extern long long   SessionPerm;
extern long long   SessionId;
extern long long   UserId;
extern long long   SessionLastAcc;

extern time_t      HackTime;

extern char      * linkfile_name;
extern char      * IntraScript;

void usage(int code);
int  access_update();

int acc_transaction  (accbase_t * base, logbase_t * logbase, int accno, is_data_t * isdata, double realarg, double limit);
int acc_charge_trans (accbase_t * base, logbase_t * logbase, int accno, is_data_t * isdata);

int acc_bitrans      (accbase_t * base, logbase_t * logbase, int accno, is_data_t * isdata, u_int val_in, u_int val_out, double realarg, double limit);

int     accs_state(acc_t * acc);
double  acc_limit (acc_t * acc);

#endif /* __CORE_H__ */
