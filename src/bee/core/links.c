/* $RuOBSD: links.c,v 1.5 2002/10/24 10:31:43 shadow Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <bee.h>
#include <links.h>
#include <res.h>

char      * linkfile="/var/bee/reslinks.dat";
char      * linktemp="/var/bee/reslinks.tmp";
char      * linklock="/var/bee/link.lock";
reslink_t * linktab=NULL;
int         linktabsz=0;

#define DELIM  " ,\t\n\r"

int reslinks_lock(int locktag)
{  int lockfd;

   lockfd=open(linklock, O_CREAT);
   if (lockfd==-1)
   {  syslog(LOG_ERR, "reslinks_lock(open(%s)): %m", linklock);
      return (-1);
   }
   if (flock(lockfd, locktag)==-1)  
   {  syslog(LOG_ERR, "reslinks_lock(flock): %m");
      return (-1);
   }
   return lockfd;
}

int reslinks_unlock(int lockfd)
{
   if (flock(lockfd, LOCK_UN)==-1)
   {  syslog(LOG_ERR, "reslinks_unlock(flock): %m");
      return (-1);
   }
   close(lockfd);
   return 0;
}

int reslinks_load(int locktag)
{  FILE      * fd;
   char        buf[128];
   char      * ptr;
   char      * str;
   reslink_t   worksp;
   void      * tmp;
   int         lin=0;
   int         i;
   int         fdlock=-1;

   if (locktag != LOCK_UN) 
   {  fdlock=reslinks_lock(locktag);
      if (fdlock == -1) return (-1);
   }  

   if (linktab != NULL) 
   {  free(linktab);
      linktab=NULL;
   }
   linktabsz=0;

   fd=fopen(linkfile, "r");
   if (fd == NULL)
   {  syslog(LOG_ERR, "reslinks_load(fopen(%s)): %m", linkfile);
      if (fdlock != -1) reslinks_unlock(fdlock);
      return (-1);
   }
   while(fgets(buf, sizeof(buf), fd) != NULL)
   {  lin++;
// comment check
      if (*buf == '#') continue;
      ptr=buf;
// stub for gate allowance
      worksp.allow=1;
// get resource name
      str=next_token(&ptr, DELIM);
      if (str==NULL) continue;  // skip empty line
// tranlate res.name -> res.id
      for (i=0; i<resourcecnt; i++)
        if (strcmp(resource[i].name, str)==0) break;
      if (i==resourcecnt) 
      {  syslog(LOG_ERR, "%s: %d: Unknown resource name \"%s\"", 
                 linkfile, lin, str);  
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
      {  syslog(LOG_ERR, "%s: %d: Unexpected line end", linkfile, lin);
         continue;
      }  
// Get username/address 
      str=next_token(&ptr, DELIM);
      if (str==NULL)
      {  syslog(LOG_ERR, "%s: %d: Unexpected line end", linkfile, lin);
         continue;
      }
      tmp=calloc(1, strlen(str)+1);
      if (tmp == NULL)
      {  syslog(LOG_ERR, "reslinks_load(calloc): %m");
         fclose(fd);
         if (fdlock != -1) reslinks_unlock(fdlock);
         return (-1);
      }
      worksp.username=(char*)tmp;
      strcpy(worksp.username, str);
      tmp=realloc(linktab, (linktabsz+1)*sizeof(reslink_t));
      if (tmp == NULL)
      {  syslog(LOG_ERR, "reslinks_load(realloc): %m");
         fclose(fd);
         if (fdlock != -1) reslinks_unlock(fdlock);
         return (-1);
      }
      linktab=(reslink_t*)tmp;
      linktab[linktabsz++]=worksp;
   } // while()
   fclose(fd);
   if (fdlock != -1) reslinks_unlock(fdlock);
   return 0;
}

int reslinks_save(int locktag)
{  FILE * fd;
   int    lockfd=-1;
   int    i;
   int    rc;

   if (locktag != LOCK_UN) 
   {  lockfd=reslinks_lock(locktag);
      if (lockfd == -1) return (-1);
   }  

   fd=fopen(linktemp, "w"); 
   if (fd==NULL)
   {  syslog(LOG_ERR, "reslinks_save(fopen): %m");
      if (lockfd != -1) reslinks_unlock(lockfd);
      return -1;
   }
   rc=fprintf(fd, "# Created by bee\n"
           "#\tres\tuid\tacc\tname\n");
   if (rc >= 0)
   {  for (i=0; i<linktabsz; i++)
      {  rc=fprintf(fd, "\t%s\t%d\t%d\t%s\n",
              resource[linktab[i].res_id].name,
              linktab[i].user_id,
              linktab[i].accno,
              linktab[i].username);
         if (rc < 0) break;
      }
   }
   fclose(fd);
   if (rc < 0) 
   {  syslog(LOG_ERR, "reslinks_save(fprintf): %m");
      if (lockfd != -1) reslinks_unlock(lockfd);
      return -1;
   }
   rename(linktemp, linkfile);

   if (lockfd != -1) reslinks_unlock(lockfd);
   return 0;
}

int reslink_new(int rid, int accno, char * name)
{  int        i;
   int        uid=-1;
   reslink_t  newlink;
   void     * tmp;

// initialize new gate     
   newlink.res_id=rid;
   newlink.accno=accno;
   newlink.allow=1;
   tmp=calloc(1, strlen(name)+1);
   if (tmp==NULL)
   {  syslog(LOG_ERR, "reslink_new(calloc): %m");
      return (-1);
   }
   strcpy((char *)tmp, name);
   newlink.username=(char *)tmp;

   for(i=0; i<linktabsz; i++)
      if (linktab[i].res_id==rid && linktab[i].user_id>uid) 
          uid=linktab[i].user_id;
   uid++;
   newlink.user_id=uid;
   tmp=realloc(linktab, (linktabsz+1)*sizeof(newlink));
   if (tmp==NULL)
   {  syslog(LOG_ERR, "reslink_new(calloc): %m");
      return (-1);
   }
   linktab=(reslink_t *)tmp;
   linktab[linktabsz++]=newlink;

   return 0;
}

int reslink_del(int index)
{  void * tmp;

   if (index <0 || index>=linktabsz) return (-1);
   if (index != linktabsz-1)
      memmove(linktab+index, linktab+index+1, 
               (linktabsz-1-index)*sizeof(reslink_t));
   linktabsz--;
   if (linktabsz == 0) linktab=NULL;
   else
   {  tmp=realloc(linktab, linktabsz*sizeof(reslink_t));
      if (tmp == NULL)
         syslog(LOG_ERR, "reslink_del(realloc): %m");
      else
         linktab=(reslink_t*)tmp;
   }

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
   if (str != NULL) *bits=strtol(str, NULL, 10);
   else *bits=32;
   *addr &= make_addr_mask(*bits);
   return 0;
}

unsigned long make_addr_mask(int bits)
{  return swap32(~((1L << (32-bits))-1));
}   
   

