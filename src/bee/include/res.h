/* $RuOBSD$ */

#ifndef __RES_H__
#define __RES_H__

#define RES_INET    0
#define RES_MAIL    1
#define RES_ADDER   2

typedef money_t (*count_proc_t)(is_data_t * data);

struct _resource_t
{  int           id;
   count_proc_t  count;
   char        * name;
   char        * ruler_cmd;
   int		 fAddr;
};

extern resource_t   resource[];  // static resource table
extern int          resourcecnt; // resources count

money_t inet_count_proc(is_data_t * data);
money_t mail_count_proc(is_data_t * data);
money_t adder_count_proc(is_data_t * data);

#endif /* __RES_H__ */
