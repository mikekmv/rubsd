/* $RuOBSD$ */
#include <string.h>
#include <stdlib.h>

#include <bee.h>

char * next_token (char ** ptr, char * delim)
{  char * str;

   do
   {  str=strsep(ptr, delim);
      if (str == NULL) break;
   } while (*str=='\0');
   return str;
}
