/* $oganer: da.c,v 1.2 2003/06/09 11:08:51 tm Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include "da.h"

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
