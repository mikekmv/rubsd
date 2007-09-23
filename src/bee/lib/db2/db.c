#include <stdio.h>
#include <stdlib.h>

#define __PGSQL
#include <sql.h>

#include <lists.h>
#include <bee.h>
#include <db2.h>
#include "internal.h"

int DumpQuery = 0;  // Dump query before execute (debug)

/* * * * * * * * * * * * * * * * * * *\
 *     Initialize libsql data &      * 
 *  store some values for sql_open() *
\* * * * * * * * * * * * * * * * * * */

void *   db2_init      (char * type, char * server, char * dbname)
{  int                sqltype;
   struct sqldata   * sqldata;
   dbintdata_t      * intdata;

   sqltype = sql_type(type);
   if (sqltype < 0) return NULL;

   sqldata = sql_init(NULL, sqltype);
   if (sqldata == NULL) return NULL;

   INTDATA(sqldata) = calloc(1, sizeof(dbintdata_t));
   if (INTDATA(sqldata) == NULL) 
   {  free(sqldata);   // STUB
      return NULL;
   }

   intdata = INTDATA(sqldata);

   intdata->server = server;
   intdata->dbname = dbname;
   intdata->rows   = (-1);
   intdata->cols   = (-1);

   return sqldata;
}

/* * * * * * * * * * * * * * * * * * *\
 *     Open database connection      * 
 *    w/specific name & password     *
\* * * * * * * * * * * * * * * * * * */

int      db2_open      (void * data, char * module, char * code)
{  struct sqldata *  sqldata;
   dbintdata_t    * intdata;
   int                rc;

   if (data == NULL) return (-1);
   if (SQLDATA(data)->data == NULL) return (-1);

   sqldata = SQLDATA(data);
   intdata = INTDATA(data);

   rc = sql_open(sqldata, intdata->server, intdata->dbname, module, code);
   if (rc != SQL_SUCCESS) return (-1);

   return 0;
}

/* * * * * * * * * * * * * * * * * * *\
 *    Close database connection      * 
\* * * * * * * * * * * * * * * * * * */

int      db2_close     (void * data)
{  struct sqldata  * sqldata;
   int               rc;

   if (data == NULL) return (-1);
   if (SQLDATA(data)->data == NULL) return (-1);

   sqldata = SQLDATA(data);

   rc = sql_close(sqldata);
   if (rc != SQL_SUCCESS) return (-1);
  
   return 0;
}

char **  db2_search    (void * data, int max, char * format, ...)
{  struct sqldata   * sqldata;
   dbintdata_t      * intdata;
   int                rc;
   char            *  buf;
   va_list            valist;

   if (data == NULL || format == NULL) return NULL;
   if (SQLDATA(data)->data == NULL) return NULL;

   sqldata = SQLDATA(data);
   intdata = INTDATA(data);

   buf = calloc(1, SQLD_QBUFFSZ);
   if (buf == NULL) return NULL;

   va_start(valist, format);
   vsnprintf(buf, SQLD_QBUFFSZ, format, valist);
   va_end(valist);

   if (DumpQuery)
      fprintf(stderr, "Query dump: <<%s>>\n", buf);

   rc = sql_prepare(sqldata, "%s", buf);
   free(buf);
   if (rc != SQL_SUCCESS) return NULL;
   
   rc = sql_execute(sqldata);
   if (rc != SQL_SUCCESS) return NULL;
 
   intdata->cols = sqldata->cols;
   if (max <= 0) intdata->rows = sqldata->rows;
   else intdata->rows = MIN(sqldata->rows, max);

   rc = sql_fetch(sqldata, &(intdata->tuple), 
           max > 0 ? max : SQL_ALLROWS, 1);     // STUB: no buffering
   if (rc != SQL_SUCCESS)
   {  sql_finish(sqldata);
      return NULL;
   }
      
   pl_add(&(intdata->tuple->data), NULL);    // add terminator

   return (char**)(intdata->tuple->data.ptr);
}

int      db2_howmany   (void * data)
{ 
   if (data == NULL) return (-1);
   if (SQLDATA(data)->data == NULL) return (-1);

   return INTDATA(data)->rows;
}

char **  db2_next      (void * data)
{  struct sqldata   * sqldata;
   dbintdata_t      * intdata;
   int                rc;

   if (data == NULL) return NULL;
   if (SQLDATA(data)->data == NULL) return NULL;

   sqldata = SQLDATA(data);
   intdata = INTDATA(data);

   rc = sql_fetchnext(sqldata, &(intdata->tuple));     // STUB: no buffering
   if (rc != SQL_SUCCESS)  return NULL;
      
   pl_add(&(intdata->tuple->data), NULL);    // add terminator

   return (char**)(intdata->tuple->data.ptr);
}

int      db2_endsearch (void * data)
{  struct sqldata   * sqldata;
   dbintdata_t      * intdata;

   if (data == NULL) return (-1);
   if (SQLDATA(data)->data == NULL) return (-1);

   sqldata = SQLDATA(data);
   intdata = INTDATA(data);

   sql_finish(sqldata);

   if (intdata->tuple != NULL)
   {  sql_freetuple(intdata->tuple);
      free(intdata->tuple);
      intdata->tuple = NULL;
   }

   intdata->cols  = (-1);
   intdata->rows  = (-1);
   
   return 0;   
}

int      db2_execute   (void * data, char * format, ...)
{  struct sqldata   * sqldata;
   dbintdata_t      * intdata;
   int                rc;
   char            *  buf;
   va_list            valist;

   if (data == NULL || format == NULL) return (-1);
   if (SQLDATA(data)->data == NULL) return (-1);

   sqldata = SQLDATA(data);
   intdata = INTDATA(data);

   buf = calloc(1, SQLD_QBUFFSZ);
   if (buf == NULL) return (-1);

   va_start(valist, format);
   vsnprintf(buf, SQLD_QBUFFSZ, format, valist);
   va_end(valist);

   if (DumpQuery)
      fprintf(stderr, "Query dump: <<%s>>\n", buf);

   rc = sql_prepare(sqldata, "%s", buf);
   free(buf);
   if (rc != SQL_SUCCESS) return (-1);
   
   rc = sql_run(sqldata);
   if (rc != SQL_SUCCESS) return (-1);

   sql_finish(sqldata);

   return 0;
}

char *  db2_strescape (void * data, char * dst, char * src, int len)
{  struct sqldata   * sqldata;
   int                rc;

   if (data == NULL || dst == NULL || src == NULL || len == 0) return NULL;

   sqldata = SQLDATA(data);

   rc = sql_strescape(sqldata, dst, src, len);
   if (rc < 0) return NULL;
   
   return dst;
}
