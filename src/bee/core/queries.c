#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <bee.h>
#include <db2.h>

#include "queries.h"

int        cnt_qlist = 0; 
query_t  * itm_qlist = NULL;

char * query_ptr (char * name)
{  int   i;

   if (name == NULL) return NULL;

   for (i=0; i < cnt_qlist; i++)
      if (strcasecmp(itm_qlist[i].name, name) == 0) return itm_qlist[i].query;

   return NULL;
}

void qlist_free()
{  int i;

   for (i=0; i < cnt_qlist; i++)
   {  free(itm_qlist[i].name);
      free(itm_qlist[i].query);
   }

// do da_empty job (speed hack)
   if (itm_qlist != NULL)
   {  free(itm_qlist);
      itm_qlist = NULL;
   }
   cnt_qlist = 0; 
}

char lbuf[256];

int qlist_load(char * filename)
{  int        line = 1;
   FILE     * f;
   int        len;
   int        rc;
   int        mode = 0;
   char     * ptr;
   char     * str;
   int        qsz = 0;
   void     * tmp;
   query_t    item = {NULL, NULL};

   f = fopen(filename, "r");
   if (f == NULL)
   {  syslog(LOG_ERR, "%s: %m", filename);
      return (-1);
   }

   while(fgets(lbuf, sizeof(lbuf), f) != NULL)
   {  len = strlen(lbuf);
      if (len == (sizeof(lbuf)-1)      &&
          lbuf[sizeof(lbuf)-2] != '\n' &&
          lbuf[sizeof(lbuf)-2] != '\r')
      {  syslog(LOG_ERR, "%s:%d: Line too long", filename, line);
         return (-1);
      }
      if (mode == 0)
      {  switch(*lbuf)
         {  case '<':
               mode = 1;
               ptr = lbuf + 1;
               str = next_token(&ptr, " \t\n\r");
               if (str == NULL) 
               {  syslog(LOG_ERR, "%s:%d: (Warning) Unnamed query", 
                         filename, line);
               }
               else
               {  rc = asprintf(&(item.name), "%s", str);
                  if (rc < 0)
                  {  syslog(LOG_ERR, "qlist_load(asprintf): %m");
                     return (-1);
                  }
                  item.query = NULL;    // initialize query
                  qsz = 0;
               }
               break;
            case '>':
               syslog(LOG_ERR, "%s:%d: Unexpected '>'", filename, line);
               return (-1);
         }
      }
      else
      {  switch(*lbuf)
         {  case '>':
               if (item.query == NULL)
               {  syslog(LOG_ERR, "%s:%d: Empty query", filename, line);
                  return (-1);
               }
               rc = da_ins(&cnt_qlist, &itm_qlist, sizeof(item), (-1), &item);
               if (rc < 0)
               {  syslog(LOG_ERR, "qlist_load(da_ins): %m");
                  return (-1);
               }
               item.name  = NULL;
               item.query = NULL;
               qsz        = 0;
               mode       = 0;
               break;
            case '<':
               syslog(LOG_ERR, "%s:%d: Unexpected '<'", filename, line);
               return (-1);
            default:
               tmp = realloc(item.query, qsz + len + 1);
               if (tmp == NULL)
               {  syslog(LOG_ERR, "qlist_load(realloc(%d)): %m", qsz + len + 1);
                  if (item.query != NULL) free(item.query);
                  free(item.name);
                  return (-1);
               }
               item.query = tmp;
               memcpy(item.query + qsz, lbuf, len + 1);
               qsz += len;
         }
      }
      line++;
   }

   fclose(f);

   return 0;
}

int     query_vcatstr(char * buf, int size, char * str, va_list vl)
{
   int      cnt_args = 0;     // arg cache count
   char   * itm_args[9];      // arg cache
   char   * ptr  = str;
   char   * ptr2;
   int      len = 0;
   int      n;
   int      m;

   while ( (ptr2 = strchr(ptr, '$')) != NULL)
   {  n = ptr2[1];
      if (n >= '1' &&  n <= '9')
      {  n -= '0';

         while (cnt_args < n)
         {  itm_args[cnt_args++] = va_arg(vl, char*);
         }
         
      }
      else 
      {  n = 0;
         ptr2++;  // include '$'
      }

      m = (u_long)ptr2 - (u_long)ptr;
      if (m > size - len) m = size - len;

      if (m > 0)
      {  memcpy(buf+len, ptr, m);
         buf[len+m+1] = '\0';
         len += m;
      }

      if (n > 0)
      {  len += snprintf(buf+len, size - len, "%s", itm_args[n-1]);
         ptr2 += 2;
      }
      ptr = ptr2;
   }

   len += snprintf(buf+len, size - len, "%s", ptr);

   return len;
}

int     query_catstr(char * buf, int size, char * str, ...)
{  va_list  vl;
   int      rc;

   if (buf == NULL || size < 2) return (-1);
   if (str == NULL) return 0;

   va_start(vl, str);
   rc = query_vcatstr(buf, size, str, vl);  
   va_end(vl);

   return rc;
}

int     query_cat (char * buf, int size, char * name, ...)
{  char *   str;
   va_list  vl;
   int      rc;

   if (buf == NULL || size < 2 || name == NULL) return (-1);

   str = query_ptr(name);
   if (str == NULL) return 0;

   va_start(vl, name);
   rc = query_vcatstr(buf, size, str, vl);
   va_end(vl);

   return rc;
}

static char dbexbuf[65536];

char **  dbex_search  (void * data, int max, char * name, ...)
{  char *   str;
   va_list  vl;
   int      rc;

   if (name == NULL) return NULL;

   str = query_ptr(name);
   if (str == NULL) return NULL;

   va_start(vl, name);
   rc = query_vcatstr(dbexbuf, sizeof(dbexbuf), str, vl);
   va_end(vl);

   if (rc < 0) return NULL;

   return db2_search(data, max, "%s", dbexbuf);
}

int      dbex_execute (void * data, char * name, ...)
{  char *   str;
   va_list  vl;
   int      rc;

   if (name == NULL) return (-1);

   str = query_ptr(name);
   if (str == NULL) return (-1);

   va_start(vl, name);
   rc = query_vcatstr(dbexbuf, sizeof(dbexbuf), str, vl);
   va_end(vl);

   if (rc < 0) return (-1);

   return db2_execute(data, "%s", dbexbuf);
}
