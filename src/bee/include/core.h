/* $Bee$ */
 
#ifndef __CORE_H__
#define __CORE_H__

#define OPTS "hA:udc"

extern char * __progname;

extern logbase_t   Logbase;
extern accbase_t   Accbase;
extern link_t    * ld;
extern int         NeedUpdate;
extern int         HumanRead;
extern int         MachineRead;

void usage(int code);
int  access_update();

#endif /* __CORE_H__ */
