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

int da_swap (int * cnt, void * array, int size, int ind1, int ind2)
{
   return memswap(da_ptr(cnt, array, size, ind1), 
                  da_ptr(cnt, array, size, ind2), size);
}


int memswap(void * ptr1, void * ptr2, int size)
{  int dwords;
   int i;

   if (ptr1 == NULL || ptr2 == NULL || size == 0) return (-1);

   dwords = size / sizeof(long);

   for (i=0; i < dwords; i++)
      SWAP(((unsigned long *)ptr1)[i], ((unsigned long *)ptr2)[i]);

   for (i*=sizeof(long); i < size; i++)
      SWAP(((unsigned char *)ptr1)[i], ((unsigned char *)ptr2)[i]);

   return 0;
}

int da_reverse(int * cnt, void * array, int size)
{  int b, e;

   if (cnt == NULL) return (-1);

   for (b = 0, e = *cnt; b < e; b++, e--)
      if (da_swap(cnt, array, size, b, e) < 0) return (-1);

   return 0;
}

/*
int da_bsort(int * cnt, void * array, int size, cmpfunc_t func)
{  
   int i, d, n;

   if (cnt  == NULL || func == NULL) return (-1);

   do
   {  for (i)


   } while(n > 0);

   return 0;
}
*/
