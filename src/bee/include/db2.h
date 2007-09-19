/* $RuOBSD: db.h,v 1.6 2004/11/28 17:30:54 shadow Exp $ */
#ifndef __DB_H__
#define __DB_H__

#include <stdarg.h>

// User/commands permissions
#define PERM_NONE       0           /* unprivileged command */
#define PERM_VIEW       1           /* permission to view DB          */
#define PERM_PAY        2           /* permission to perform payments */
#define PERM_WRITE      4           /* permission to alter DB         */

#define PERM_SUPERUSER  (2147483648u)  /* permission to do anything      */

extern int DumpQuery;  // Dump query before execute (debug)

__BEGIN_DECLS

void *   db_init      (char * type, char * server, char * dbname);
int      db_open      (void * data, char * module, char * code);
int      db_close     (void * data);

char **  db_search    (void * data, int max, char * format, ...);
int      db_howmany   (void * data);
char **  db_next      (void * data);
int      db_endsearch (void * data);

int      db_execute   (void * data, char * format, ...);

char *   db_strescape (void * data, char * dst, char * src, int len);

__END_DECLS

#endif /* __DB_H__ */
