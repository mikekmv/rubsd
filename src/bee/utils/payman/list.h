/* Users list header file */

#include <sys/types.h>

#define SWAP(a,b) ((a)^=(b)^=(a)^=(b))

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

};

extern USERLIST UserList;
extern char *   AccListFile;

char * next_token(char ** ptr, char * delim);

