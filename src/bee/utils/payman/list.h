/* Users list header file */

#ifndef __LIST_H__
#define __LIST_H__

#include <sys/types.h>

#define SWAP(a,b) ((a)^=(b)^=(a)^=(b))

// Update flags
#define ULF_LIGHTMOV  1
#define ULF_WINDMOV   2 
#define ULF_REFRESH   4

typedef struct
{  char * login;
   char * domain;
} maildata_t;

typedef struct
{  in_addr_t  addr;
   int        mask;
} hostdata_t; 

typedef struct
{  char * switch_id;
   int    port;
} portdata_t; 

typedef struct
{  char       * regname;
   int          inet_acc;
   int          intra_acc;
   int          cnt_mail;
   maildata_t * itm_mail;
   int          cnt_hosts;
   hostdata_t * itm_hosts;
   int          cnt_ports;
   portdata_t * itm_ports;
} userdata_t;

class USERLIST
{  public:
   int  lin;
   int  col;
   int  lins;
   int  cols;

   int  first;
   int  marked;
   int  last_marked;
   int  flags;

   int          cnt_users;
   userdata_t * itm_users;

   int          cnt_accs;
   int        * itm_accs;

// Methods

   int   load_accs(char * filespec);
   void  free_accs();
   int   load_list();
   void  free_list();

   void  initview();
   void  refresh();

   int   user_str (char * buf, int len, int index);
};

extern USERLIST UserList;
extern char *   AccListFile;

char * next_token(char ** ptr, char * delim);

#endif
