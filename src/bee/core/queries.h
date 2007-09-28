#ifndef __QUERIES_H__
#define __QUERIES_H__

typedef struct
{  char   * name;
   char   * query;
} query_t;

extern int        cnt_qlist;
extern query_t  * itm_qlist;

int     qlist_load(char * filename);
void    qlist_free();

char *  query_ptr   (char * name);
int     query_cat   (char * buf, int size, char * name, ...);
int     query_catstr(char * buf, int size, char * str, ...);

char **  dbex_search  (void * data, int max, char * name, ...);
int      dbex_execute (void * data, char * name, ...);

#endif /*__QUERIES_H__*/

