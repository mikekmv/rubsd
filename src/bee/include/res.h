/* $RuOBSD: res.h,v 1.14 2007/09/25 14:49:01 shadow Exp $ */

#ifndef __RES_H__
#define __RES_H__

#define RES_INET     0
#define RES_MAIL     1
#define RES_ADDER    2
#define RES_INTRA    3
#define RES_LIST     4
#define RES_LOGIN    5
#define RES_LABEL    6
#define RES_MAXRATE  7

typedef double (*count_proc_t)(is_data_t * data, acc_t * acc);
typedef double (*charge_proc_t)(acc_t * acc);

typedef struct
{  int            id;
   count_proc_t   count;
   charge_proc_t  charge;
   char         * name;
   char         * ruler_cmd;
   int            fAddr;
} resource_t;

extern resource_t   resource[];  // static resource table
extern int          resourcecnt; // resources count

__BEGIN_DECLS

// core functions
void res_coreinit();

double  inet_count_proc (is_data_t * data, acc_t * acc);
double  inet_charge_proc(acc_t * acc);

double  mail_count_proc (is_data_t * data, acc_t * acc);
double  adder_count_proc(is_data_t * data, acc_t * acc);
double  intra_count_stub(is_data_t * data, acc_t * acc);
double  charge_count_proc(is_data_t * data, acc_t * acc);

__END_DECLS

#endif /* __RES_H__ */
