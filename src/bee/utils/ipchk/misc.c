/* $oganer: misc.c,v 1.1 2002/11/18 02:58:35 shadow Exp $ */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include "misc.h"

int da_empty(int * cnt, void * array, int size)
{
   if (array == NULL || cnt == NULL) return (-1);
   if (*(void**)array != NULL)
   {  free(*(void**)array);
      *(void**)array=NULL;
   }
   *cnt=0;

   return 0;
}

void * da_new  (int * cnt, void * array, int size, int ind)
{  void * tmp;

   if (array == NULL || cnt == NULL || size == 0) return NULL;
   if (ind == (-1)) ind = *cnt;
   if (ind > *cnt || ind < 0) return NULL;
   tmp=realloc(*(void**)array, (*cnt+1)*size);
   if (tmp==NULL) return NULL;
   *(void**)array=tmp;
   (*cnt)++;
   tmp=(void*)(((char*)*(void**)array)+(ind*size)); // ptr to new item
   if (ind != *cnt-1)                       // if not last
      memmove( ((char*)tmp)+size, tmp, (*cnt-ind-1)*size);
   memset(tmp, 0, size);

   return tmp;
}

int    da_ins	(int * cnt, void * array, int size, int ind, void * src)
{  void * tmp;

   if (src == NULL) return (-1);
   tmp=da_new(cnt, array, size, ind);
   if (tmp==NULL) return (-1);
   memmove(tmp, src, size);
// return new member number
	if (ind == -1) return (*cnt)-1;
	else return ind;
}

int    da_rm   (int * cnt, void * array, int size, int ind, void * dst)
{   void * tmp;

   if (array == NULL || cnt == NULL || size == 0) return (-1);
   if (ind >= *cnt || ind < 0) return (-1);
   tmp=(void*)(((char*)*(void**)array)+(ind*size));
   if (dst != NULL) memmove(dst, tmp, size);
   if (ind != (*cnt-1))
      memmove(tmp, ((char*)tmp)+size, (*cnt-ind-1)*size);
   if (*cnt > 1)
   {  tmp=realloc(*(void**)array, (*cnt-1)*size);
      if (tmp==NULL) return (-1);
      *(void**)array=tmp;
   }
   else
   {  free(*(void**)array);
      *(void**)array=NULL;
   }
   (*cnt)--;
   return 0;
}


void * da_ptr	(int * cnt, void * array, int size, int ind)
{
   if (array == NULL || cnt == NULL || size == 0) return NULL;
   if (ind >= *cnt || ind < 0) return NULL;
   return (void*)(((char*)*(void**)array)+(ind*size)); // ptr to item
}

int    da_put	(int * cnt, void * array, int size, int ind, void * src)
{  void * tmp;

   if (src == NULL) return (-1);
   tmp=da_ptr(cnt, array, size, ind);
   if (tmp==NULL) return (-1);
   memmove(tmp, src, size);

   return 0;
}

int da_get (int * cnt, void * array, int size, int ind, void * dst)
{  void * tmp;

   if (dst == NULL) return (-1);
   tmp=da_ptr(cnt, array, size, ind);
   if (tmp==NULL) return (-1);
   memmove(dst, tmp, size);

   return 0;
}

char * next_token(char ** ptr, char * delim)
{  char * str;

   do
   {  str=strsep(ptr, delim);
      if (str==NULL) break;
   } while(*str=='\0');

   return str;
}


char * strnalloc(char * str, int len)
{  char * buf;

   if (str == NULL) return NULL;
   if (len == 0) len=strlen(str);
   buf = calloc(1, len+1);
   if (buf != NULL) memcpy(buf, str, len);
   else syslog(LOG_ERR, "strnalloc(calloc): %m");

   return buf;
}


#ifdef NEVER_COMPILE
time_t timegm(struct tm * stm)
{  struct tm   zstm;
   time_t      timet;

   memset(&zstm, 0, sizeof(zstm));
   zstm.tm_year  = 70;
   zstm.tm_mday  = 1;
   zstm.tm_isdst = 0;

   stm->tm_isdst   = 0;

   timet  = mktime(stm);
   timet -= mktime(&zstm);

   return timet;
}
#endif

// Get shifted (to local) time & microseconds
void  get_time(time_t * ptime, int * pslice)
{  struct timeval stv;

   gettimeofday(&stv, NULL);
   *ptime  = shift_time(stv.tv_sec);
   *pslice = stv.tv_usec;
}

// Shift time to local
time_t shift_time(time_t utc)
{  struct tm     stm;

   localtime_r (&utc, &stm);
   return utc + stm.tm_gmtoff;
}


// Get current time
void newget_time(struct timeval * tv)
{
   gettimeofday(tv, NULL);
}
