#ifndef __USERVIEW_H__
#define __USERVIEW_H__

#include <bee.h>
#include <db.h>

#include "list.h"

#define UVF_REFRESH       1
#define UVF_ACC1_LOADED   0x10
#define UVF_ACC2_LOADED   0x20

class USERVIEW
{  public:

   userdata_t * user;          // DO NOT free() !
   int          flags;

   int          accflag1;
   int          accflag2;
   acc_t        accstate1;
   acc_t        accstate2;   

   int  load_accs();
   int  refresh();
};

extern USERVIEW  UserView;



#endif /*__USERVIEW_H__*/

