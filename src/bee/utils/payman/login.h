#ifndef __LOGIN_H__
#define __LOGIN_H__

#define AL_NONE     0
#define AL_VIEW     1
#define AL_PAYS     2
#define AL_MASTER   3
#define AL_TOTAL    4
#define AL_LAST     (AL_TOTAL)

#define ALEVELS   (AL_LAST + 1)

typedef struct
{  char * name;
   char * passhash;
   int    level;
} login_t;

extern char *    al_idents[];

extern int       cnt_logins;
extern login_t * itm_logins;

extern char    loggeduser[];

extern char    lastlogin[];
extern char    lastpass[];

int LogInUser();

int logins_load    ();
int logins_destroy ();

int login_check   (char * login, char * pass);

#endif /*__LOGIN_H__*/

