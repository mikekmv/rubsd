/* $Bee$ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <bee.h>
#include <links.h>
#include <res.h>

reslink_t * linktab=NULL;
int         linktabsz=0;

#define DELIM  " ,\t\n\r"

int reslinks_load(char * file)
{  FILE      * fd;
   char        buf[128];
   char      * ptr;
   char      * str;
   reslink_t   worksp;
   void      * tmp;
   int         lin=0;
   int         i;  

   if (linktab != NULL) 
   {  free(linktab);
      linktab=NULL;
   }
   linktabsz=0;
   fd=fopen(file, "r");
   if (fd == NULL)
   {  syslog(LOG_ERR, "reslinks_load(fopen): %m");
      return (-1);
   }
   while(fgets(buf, sizeof(buf), fd) != NULL)
   {  lin++;
// comment check
      if (*buf == '#') continue;
      ptr=buf;
// get resource name
      str=next_token(&ptr, DELIM);
      if (str==NULL) continue;  // skip empty line
// tranlate res.name -> res.id
      for (i=0; i<resourcecnt; i++)
        if (strcmp(resource[i].name, str)==0) break;
      if (i==resourcecnt) 
      {  syslog(LOG_ERR, "%s: %d: Unknown resource name \"%s\"", 
                 file, lin, str);  
         continue;
      }
      else worksp.res_id=i; 
// Parse 2 int values (user_id, accno)
      for (i=1; i<3; i++)
      {  str=next_token(&ptr, DELIM);
         if (str==NULL) break;
         ((int *)&worksp)[i]=strtol(str, NULL, 0);
      }
      if (i < 3)
      {  syslog(LOG_ERR, "%s: %d: Unexpected line end", file, lin);
         continue;
      }  
// Get username/address 
      str=next_token(&ptr, DELIM);
      if (str==NULL)
      {  syslog(LOG_ERR, "%s: %d: Unexpected line end", file, lin);
         continue;
      }
      tmp=calloc(1, strlen(str)+1);
      if (tmp == NULL)
      {  syslog(LOG_ERR, "reslinks_load(calloc): %m");
         fclose(fd);
         return (-1);
      }
      worksp.username=(char*)tmp;
      strcpy(worksp.username, str);
      tmp=realloc(linktab, (linktabsz+1)*sizeof(reslink_t));
      if (tmp == NULL)
      {  syslog(LOG_ERR, "reslinks_load(realloc): %m");
         fclose(fd);
         return (-1);
      }
      linktab=(reslink_t*)tmp;
      linktab[linktabsz++]=worksp;
   } // while()
   fclose(fd);
   return 0;
}

int lookup_res (int rid, int uid, int * index)
{  
   for ((*index)++; (*index)<linktabsz; (*index)++)
      if (linktab[*index].res_id==rid && linktab[*index].user_id==uid)
        return *index;
   return (-1); 
}

int lookup_resname (int rid, char * name, int * index)
{  int rc;

   for ((*index)++; (*index)<linktabsz; (*index)++)
      if (linktab[*index].res_id==rid)
      {  if (resource[rid].fAddr) 
            rc=inaddr_cmp(name, linktab[*index].username);
         else rc=strcmp(name, linktab[*index].username);
         if (rc == 0) return *index;
      } 
   return (-1); 
}


int lookup_accno (int accno, int * index)
{
   for ((*index)++; (*index)<linktabsz; (*index)++)
     if (linktab[*index].accno==accno) return *index;
   return (-1);
}

int lookup_name (char * name, int * index)
{
   for ((*index)++; (*index)<linktabsz; (*index)++)
     if (strcmp(linktab[*index].username, name)==0) return *index;
   return (-1);
}

int lookup_addr (char * addr, int * index)
{
   for ((*index)++; (*index)<linktabsz; (*index)++)
     if (inaddr_cmp(linktab[*index].username, addr)==0) return *index;
   return (-1);
}

// Compare user inet address in form n.n.n.n[/b]
// with resource link address

int inaddr_cmp(char * user, char * link)
{  unsigned long   uaddr;
   int             ubits;
   unsigned long   laddr;
   int             lbits;

   if (make_addr(link, &laddr, &lbits)==(-1)) return (-1);
   if (make_addr(user, &uaddr, &ubits)==(-1)) return (-1);
   if (ubits<lbits) return 1;   
   if (ubits>lbits) uaddr &= make_addr_mask(lbits);
   return (uaddr != laddr);  // zero if equal
}

int make_addr(const char * straddr, unsigned long * addr, int * bits)
{  char   buf[32];
   char * ptr=buf;
   char * str;

   strlcpy(buf, straddr, sizeof(buf));
   str=next_token(&ptr, "/");
   if (str==NULL) return (-1);
   if (inet_aton(str, (struct in_addr *)addr) != 1) return (-1);
   str=next_token(&ptr, "/");
   if (str != NULL) *bits=strtol(str, NULL, 0);
   else *bits=32;
   *addr &= make_addr_mask(*bits);
   return 0;
}

unsigned long make_addr_mask(int bits)
{  return swap32(~((1L << (32-bits))-1));
}   
   

