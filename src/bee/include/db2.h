/* $RuOBSD: db2.h,v 1.1 2007/09/19 04:49:20 shadow Exp $ */
#ifndef __DB2_H__
#define __DB2_H__

#include <stdarg.h>

// User/commands permissions
#define PERM_NONE       0           /* unprivileged command */
#define PERM_VIEW       1           /* permission to view DB          */
#define PERM_PAY        2           /* permission to perform payments */
#define PERM_WRITE      4           /* permission to alter DB         */

#define PERM_SUPERUSER  (2147483648u)  /* permission to do anything      */

extern int DumpQuery;  // Dump query before execute (debug)

__BEGIN_DECLS

void *   db2_init      (char * type, char * server, char * dbname);
int      db2_open      (void * data, char * module, char * code);
int      db2_close     (void * data);

char **  db2_search    (void * data, int max, char * format, ...);
int      db2_howmany   (void * data);
char **  db2_next      (void * data);
int      db2_endsearch (void * data);

int      db2_execute   (void * data, char * format, ...);

char *   db2_strescape (void * data, char * dst, char * src, int len);

__END_DECLS

#endif /* __DB2_H__ */
