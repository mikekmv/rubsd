/* $RuOBSD: res.h,v 1.8 2007/08/22 09:28:54 shadow Exp $ */

#ifndef __RES_H__
#define __RES_H__

#define RES_INET    0
#define RES_MAIL    1
#define RES_ADDER   2
#define RES_INTRA   3

typedef money_t (*count_proc_t)(is_data_t * data, acc_t * acc);
typedef money_t (*charge_proc_t)(acc_t * acc);

struct _resource_t
{  int           id;
   count_proc_t   count;
   charge_proc_t  charge;
   char        * name;
   char        * ruler_cmd;
   int		 fAddr;
};

extern resource_t   resource[];  // static resource table
extern int          resourcecnt; // resources count

money_t inet_count_proc (is_data_t * data, acc_t * acc);
money_t inet_charge_proc(acc_t * acc);

money_t mail_count_proc (is_data_t * data, acc_t * acc);
money_t adder_count_proc(is_data_t * data, acc_t * acc);
money_t intra_count_stub(is_data_t * data, acc_t * acc);
money_t charge_count_proc(is_data_t * data, acc_t * acc);

#endif /* __RES_H__ */
