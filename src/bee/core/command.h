/* $RuOBSD: command.h,v 1.4 2007/09/28 04:28:27 shadow Exp $ */

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <stdarg.h>

#define CMD_EXIT (-101)

#define CMD_DELIM " \t\n\r"

#define LF_NONE       0
#define LF_NAME       1
#define LF_ADDR       2
#define LF_ALL        3
#define LF_PNAME      4

#define ESCARG1(a)    (db2_strescape(DBdata, argbuf1, (a), sizeof(argbuf1)))
#define ESCARG2(a)    (db2_strescape(DBdata, argbuf2, (a), sizeof(argbuf2)))
#define ESCARG3(a)    (db2_strescape(DBdata, argbuf3, (a), sizeof(argbuf3)))
#define ESCARG4(a)    (db2_strescape(DBdata, argbuf4, (a), sizeof(argbuf4)))
#define ESCARG5(a)    (db2_strescape(DBdata, argbuf5, (a), sizeof(argbuf5)))
#define ESCARG6(a)    (db2_strescape(DBdata, argbuf6, (a), sizeof(argbuf5)))

typedef int (*cmd_proc_t)(char * cmd, char * args);

typedef struct _command_t command_t;
struct _command_t
{  char       * ident;
   cmd_proc_t   proc;
   u_int        pl;
   command_t  * subtab;
};

typedef struct
{  int    what;
   char * str;
   int    ind;
   int    no;
} lookup_t;

typedef struct
{  int      cnt_vals;
   char  ** itm_vals;
} tabrow_t;

typedef struct
{  int        cnt_cols;
   int      * itm_widths;
   int        cnt_rows;
   tabrow_t * itm_rows;
} outtab_t;

extern char   argbuf1[128];
extern char   argbuf2[128];
extern char   argbuf3[128];
extern char   argbuf4[128];
extern char   argbuf5[128];
extern char   argbuf6[128];

// execute (sub-)command (find & execute handler)
int cmd_exec  (char * str, command_t * table);

// prepare & respond w/ complex string
int cmd_out_begin(int err);
int cmd_out_add  (char * format, ...);
int cmd_out_end  ();
int cmd_out_abort();

int cmd_tab_begin ();
int cmd_tab_value (char * format, ...);
int cmd_tab_newrow();
int cmd_tab_end   ();
int cmd_tab_abort ();

int cmd_out   (int err, char * format, ...);
int cmd_getaccno (char ** args, lookup_t * prev);
int cmd_ptime (time_t utc, char * buf);
int cmd_add   (int accno, double sum);
int cmd_accerr(int rc);
time_t cmd_gettime(char ** arg, time_t tim);
int cmd_plogrec(logrec_t * logreg);
int cmd_pdate(time_t tim, char * buf);
int cmd_getdate(char ** pptr);

int cmdh_exit   (char * cmd, char * args);
int cmdh_notimpl(char * cmd, char * args);
int cmdh_ver    (char * cmd, char * args);
int cmdh_new    (char * cmd, char * args);
int cmdh_acc    (char * cmd, char * args);
int cmdh_freeze (char * cmd, char * args);
int cmdh_fix    (char * cmd, char * args);
int cmdh_acc    (char * cmd, char * args);
int cmdh_log    (char * cmd, char * args);
int cmdh_res    (char * cmd, char * args);
int cmdh_hres    (char * cmd, char * args);
int cmdh_add	(char * cmd, char * args);
int cmdh_update	(char * cmd, char * args);
int cmdh_human	(char * cmd, char * args);
int cmdh_lookup (char * cmd, char * args);
int cmdh_date   (char * cmd, char * args);
int cmdh_report (char * cmd, char * args);
int cmdh_delete (char * cmd, char * args);
int cmdh_new_contract (char * cmd, char * args);
int cmdh_new_name (char * cmd, char * args);
int cmdh_new_vpn  (char * cmd, char * args);
int cmdh_gate (char * cmd, char * args);
int cmdh_delgate (char * cmd, char * args);
int cmdh_intraupdate (char * cmd, char * args);
int cmdh_setstart (char * cmd, char * args);
int cmdh_lock   (char * cmd, char * args);
int cmdh_accres (char * cmd, char * args);
int cmdh_docharge(char * cmd, char * args);
int cmdh_tdump(char * cmd, char * args);
int cmdh_debug(char * cmd, char * args);

int cmdh_card           (char * cmd, char * args);
int cmdh_card_gen       (char * cmd, char * args);
int cmdh_card_list      (char * cmd, char * args);
int cmdh_card_emit      (char * cmd, char * args);
int cmdh_card_check     (char * cmd, char * args);
int cmdh_card_null      (char * cmd, char * args);
int cmdh_card_nullbatch (char * cmd, char * args);
int cmdh_card_expire    (char * cmd, char * args);
int cmdh_card_utluser   (char * cmd, char * args);

#endif /* __COMMAND_H__ */
