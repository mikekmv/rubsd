/* $RuOBSD: links.h,v 1.10 2007/09/24 12:56:21 shadow Exp $ */

#ifndef __LINKS_H__
#define __LINKS_H__

#include <fcntl.h>

typedef struct 
{  int		res_id;
   int          user_id;
   int		accno;
   char       * username;
   int          allow;
   u_int        addr;
   u_int        mask;
} reslink_t;


extern reslink_t  * linktab;     // resource links table (loaded)
extern int          linktabsz;   // no of links in table

__BEGIN_DECLS

int reslinks_lock   (int locktag);
int reslinks_load   (int locktag);
int reslinks_save   (int locktag);
int reslinks_unlock (int lockfd);

int reslink_new (int rid, int accno, char * name);
int reslink_del (int index);

int lookup_res       (int rid, int uid, int * index);
int lookup_resname   (int rid, char * name, int * index);
int lookup_accno     (int accno, int * index);
int lookup_accres    (int accno, int rid, int * index);
int lookup_name      (char * name, int * index);
int lookup_pname     (char * name, int * index);
int lookup_addr      (char * addr, int * index);
int lookup_baddr     (u_int addr, int * index);
int lookup_intersect (u_int addr, u_int mask, int * index);

int   inaddr_cmp  (char * user, char * link);
int   make_addr (const char * straddr, u_int * addr, int * bits);
int   make_addrandmask (const char * straddr, u_int * addr, u_int * mask);
u_int make_addr_mask (int bits);

__END_DECLS

#endif /* __LINKS_H__ */
