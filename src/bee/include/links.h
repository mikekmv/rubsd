/* $Bee$ */

#ifndef __LINKS_H__
#define __LINKS_H__

#include <fcntl.h>

struct _reslink_t
{  int		res_id;
   int          user_id;
   int		accno;
   char       * username;
   int		allow;
};

extern reslink_t  * linktab;     // resource links table (loaded)
extern int          linktabsz;   // no of links in table

int reslinks_lock   (int locktag);
int reslinks_load   (int locktag);
int reslinks_save   (int locktag);
int reslinks_unlock (int lockfd);

int reslink_new(int rid, int accno, char * name);
int reslink_del(int index);

int lookup_res  (int rid, int uid, int * index);
int lookup_resname (int rid, char * name, int * index);
int lookup_accno(int accno, int * index);
int lookup_name (char * name, int * index);
int lookup_addr (char * addr, int * index);


int inaddr_cmp  (char * user, char * link);
int make_addr (const char * straddr, unsigned long * addr, int * bits);
unsigned long make_addr_mask (int bits);

#endif /* __LINKS_H__ */
