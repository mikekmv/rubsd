#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <bee.h>

#include "global.h"

#define SAY  printf


char * AccListFile = "/var/bee/payman.lst";
char * LinksFile   = "/var/bee/reslinks.dat";
char * linklock    = "/var/bee/link.lock";

char   rlnk_delim[]= " \t\n\t";

char   listbuf[128];

USERLIST UserList = {0,0,0,0, 0,0,0,0, 0,NULL, 0,NULL};

/* Allocate & copy string */
char * stralloc(const char * str)
{  int    len;
   char * mem; 

   if (str == NULL) return NULL;

   len = strlen(str);
   mem = (char *) calloc(1, len+1);
   if (mem == NULL) return NULL;
   memcpy(mem, str, len);

   return mem;
}

/* Free account list */

void USERLIST::free_accs()
{  
   if (itm_accs != NULL) 
   {  free(itm_accs);
      itm_accs = NULL;
   }
   cnt_accs = 0;
}

/* Load account list from file */

char acc_delims[]=" \t\n\r,;";

int USERLIST::load_accs(char * filespec)
{  int     fd = -1;
   int     rc;
   int     bytes;
   char *  buf;
   char *  ptr;
   char *  str;
   char *  endptr;

   if (filespec == NULL)
   {  syslog(LOG_ERR, "USERLIST::load_accs(): NULL filespec");
      return (-1);
   }

   fd = open(filespec, O_RDONLY, 0700);
   if (fd < 0)
   {  syslog(LOG_ERR, "USERLIST::load_accs(open(%s)): %m", filespec);
      return (-1);
   }

   rc = ioctl(fd, FIONREAD, &bytes);
   if (rc < 0)
   {  syslog(LOG_ERR, "USERLIST::load_accs(FIONREAD): %m");
      close(fd);
      return (-1);
   }
   
   if (bytes == 0) 
   {  close(fd);
      return 0;   // Success
   }

   buf = (char *) calloc(1, bytes + 1); // allocate buffer & '\0'
   if (buf == NULL)
   {  syslog(LOG_ERR, "USERLIST::load_accs(calloc): %m");
      close(fd);
      return (-1);
   }
   
   rc = read(fd, buf, bytes);
   if (rc < bytes)
   {  syslog(LOG_ERR, "USERLIST::load_accs(read): %s", rc < 0 ? strerror(errno) : 
                           "(unexpected partial read)");
      free(buf);
      close(fd);
      return (-1);
   }
      
   ptr = buf;
   while ( (str = next_token(&ptr, acc_delims)) != NULL)
   {  rc = strtol(str, &endptr, 10);
      if (*endptr != '\0') continue;
      da_ins(&cnt_accs, &itm_accs, sizeof(*itm_accs), (-1), &rc);
   }

   free(buf);
   close(fd);

   return 0;      
}

void userdata_free(userdata_t * user)
{  int i;

// free() user name
   if (user->regname != NULL)
   {  free(user->regname);
      user->regname = NULL;
   }    
// destroy mailboxes list
   if (user->itm_mail != NULL)
   {  for (i=0; i < user->cnt_mail; i++)
      {  if (user->itm_mail[i].login != NULL)
            free(user->itm_mail[i].login);
         if (user->itm_mail[i].domain != NULL)
            free(user->itm_mail[i].domain);
      }
      da_empty(&(user->cnt_mail), &(user->itm_mail), 1);
   }
// destroy hosts list
   da_empty(&(user->cnt_hosts), &(user->itm_hosts), 1);
// destroy ports list
   if (user->itm_ports != NULL)
   {  for (i=0; i < user->cnt_ports; i++)
      {  if (user->itm_ports[i].switch_id != NULL)
            free(user->itm_ports[i].switch_id);
      }
      da_empty(&(user->cnt_ports), &(user->itm_ports), 1);
  }
}


void USERLIST::free_list()
{  int u;

   for (u=0; u < cnt_users; u++)
      userdata_free(&(itm_users[u]));  
   da_empty(&(cnt_users), &(itm_users), 1);
}



char * ridents[]=
{  "inet",       // 0
   "mail",       // 1
   "adder",      // 2
   "intra",      // 3
   NULL
};

typedef struct
{  int    res_id;
   int    accno;
   char * str;
} gateitm_t;

void gatelist_free(int * pcnt_gates, gateitm_t ** pitm_gates)
{  int i;

// free() gate strings
   for (i=0; i < *pcnt_gates; i++)
   {  if ((*pitm_gates)[i].str != NULL)
      {  free((*pitm_gates)[i].str);
         (*pitm_gates)[i].str = NULL;
      }
   }
// destroy gatelist
   da_empty(pcnt_gates, pitm_gates, sizeof(gateitm_t));
}

int USERLIST::load_list()
{  int         lockfd;
   FILE      * f;
   char        buf[128];
   char      * ptr;
   char      * str;
   int         i;
   int         rc;
   gateitm_t   item;
   userdata_t  uitem;
   hostdata_t  hitem;
   maildata_t  mitem;
   portdata_t  pitem;

   int         cnt_gates = 0;;
   gateitm_t * itm_gates = NULL;

   free_list();

// Obtain shared lock to links file
   lockfd = open(linklock, O_CREAT);
   if (lockfd < 0)
   {  syslog(LOG_ERR, "USERLIST::load_list(open(lockfile %s)): %m", linklock);
      return (-1);
   }
   if (flock(lockfd, LOCK_SH) == -1)  
   {  syslog(LOG_ERR, "USERLIST::load_list(flock): %m");
      close(lockfd);
      return (-1);
   }

// Open links file
   f = fopen(LinksFile, "r");
   if (f == NULL)
   {  syslog(LOG_ERR, "USERLIST::load_list(fopen(%s)): %m", LinksFile);
      close(lockfd);
      return (-1);
   }

// Parse cycle
   while(fgets(buf, sizeof(buf), f) != NULL)
   {
// skip comment
      if (*buf == '#') continue;
      ptr = buf;
// get resource name
      str = next_token(&ptr, rlnk_delim);
      if (str == NULL) continue;  // skip empty line
// tranlate res.name -> res.id
      for (i = 0; i < 4; i++)
         if (strcmp(ridents[i], str) == 0) break;
      if (i > 3) continue;        // skip invalid line
      item.res_id = i; 
// resource gate id (skipping)
      str = next_token(&ptr, rlnk_delim);
      if (str == NULL) continue;  // skip invalid line
// account number
      str = next_token(&ptr, rlnk_delim);
      if (str == NULL) continue;  // skip invalid line
      item.accno = strtol(str, NULL, 10);

// skip not allowed accounts
      for (i=0; i < cnt_accs; i++)
         if (itm_accs[i] == item.accno) break;

      if (i >= cnt_accs) continue;

// gate string 
      str = next_token(&ptr, rlnk_delim);
      if (str == NULL) continue;  // skip invalid line 

      item.str = stralloc(str);
      if (item.str == NULL)
      {  syslog(LOG_ERR, "USERLIST::load_list(strnalloc(gate str)): %m");
         // destroy gate list
         gatelist_free(&cnt_gates, &itm_gates);
         // close files 
         fclose(f);
         close(lockfd);
         return (-1);
      }

// add gate to list
      rc = da_ins(&cnt_gates, &itm_gates, sizeof(item), (-1), &item);
      if (rc < 0)
      {  syslog(LOG_ERR, "USERLIST::load_list(da_ins(gate)): Error");
         // destroy gate item 
         free(item.str); 
         // destroy date list 
         gatelist_free(&cnt_gates, &itm_gates);
         // close files
         fclose(f);
         close(lockfd);
         return (-1);
      }
      item.str = NULL; // to avoid free()
   }

// close & unlock
   fclose(f);
   close(lockfd);
   
//for (i=0; i < cnt_gates; i++)
//SAY("%6s  %3d  \"%s\"\n", ridents[itm_gates[i].res_id],
//      itm_gates[i].accno, itm_gates[i].str);


// Lookup Users
   while(1)
   {  
//SAY("----- LOOP -----\n");
// 1. Get any adder gate (store user name, first account)
//SAY("(gates: %d) looking for any adder\n", cnt_gates);
   // find
      for (i=0; i < cnt_gates; i++)
         if (itm_gates[i].res_id == 2) break; 
      if (i >= cnt_gates)     // i.e. no adder gates left, quit
      {    
//SAY("not found - JOB DONE\n");
         break;  
      }
//SAY("found \"%s\" #%d, cutting out\n", itm_gates[i].str, itm_gates[i].accno);
   // cut gate
      rc = da_rm(&cnt_gates, &itm_gates, sizeof(item), i, &item);
      if (rc < 0)
      {  
//SAY("****** SYSTEM ERROR - ABORT\n");
         syslog(LOG_ERR, "USERLIST::load_list(da_rm(any adder)): Error");
         // drop gate list
         gatelist_free(&cnt_gates, &itm_gates);
         // free UserList
         free_list();
         return (-1);
      }

   // initialize user item
      memset(&uitem, 0, sizeof(uitem));
      uitem.intra_acc = (-1);          // i.e. second accout is n/a by default

   // get initial userlist item data from gate
      uitem.inet_acc = item.accno;
      uitem.regname  = item.str;     // dynamically allocated
      item.str = NULL;               // to avoid free()                  

// 2. Find & get same named adder gates.
//    If more then 1 gate found - drop it & drop user

//SAY("(gates: %d) looking for \"%s\" adder\n", cnt_gates, uitem.regname);

      for (i=0; i < cnt_gates; i++)
      {  if (itm_gates[i].res_id == 2 &&
             strcmp(itm_gates[i].str, uitem.regname) == 0)
         {  
//SAY("found \"%s\" #%d, cutting out\n", itm_gates[i].str, itm_gates[i].accno);

            rc = da_rm(&cnt_gates, &itm_gates, sizeof(item), i, &item);
            if (rc < 0)
            {  syslog(LOG_ERR, "USERLIST::load_list(da_rm(named adder)): Error");
//SAY("****** SYSTEM ERROR - ABORT\n");
               // drop user item
               userdata_free(&uitem);   
               // drop gate list
               gatelist_free(&cnt_gates, &itm_gates);
               // free UserList
               free_list();
               return (-1);
            }

            if (uitem.intra_acc == (-1))
            {  
//SAY("storing second account\n");
               uitem.intra_acc = item.accno;  // store second account
            }
            else
            {  
//SAY("extra account ! drop accounts\n");
                  uitem.intra_acc = (-2);     // user drop flag
            }
            free(item.str);  // free string
            item.str = NULL; // to avoid free()
         }
      }
      if (uitem.intra_acc == (-2))
      {  syslog(LOG_ERR, "USERLIST::load_list():" 
                "Warning, extra adder gates for \"%s\" - user dropped",
                uitem.regname);
//SAY("drop flag set - drop user\n");
         userdata_free(&uitem); 
         continue;  // next User
      }

// 3. Determine accounts usage (inet/intra)
//     inet/mail gates (0/1) = inet
//     intera gate     (3)   = intra
//     no gates              = undefined

      rc = 0;

//SAY("determine accounts usage\n");
// find gates & mark account usage
      for (i=0; i < cnt_gates; i++)
      {  if (uitem.inet_acc > 0 && itm_gates[i].accno == uitem.inet_acc)
         {  switch (itm_gates[i].res_id)  
            {  case 0: // inet
               case 1: // mail
                  rc |= 1;
                  break;
               case 3: // intra
                  rc |= 2;
                  break;
            }
         }
         if (uitem.intra_acc > 0 && itm_gates[i].accno == uitem.intra_acc)
         {  switch (itm_gates[i].res_id)  
            {  case 0: // inet
               case 1: // mail
                  rc |= 4;
                  break;
               case 3: // intra
                  rc |= 8;
                  break;
            }
         }
      }   

//SAY("USAGE: acc1(#%d): %d acc2(#%d):%d\n", uitem.inet_acc, (rc&3), uitem.intra_acc, (rc&12)>>2);

// drop invalid cases
      if (rc == 0 || (rc & 3) == 3 || (rc & 12) == 12 || (rc & 3) == ((rc & 12)>>2))
      {  syslog(LOG_ERR, "USERLIST::load_list(): "
                "Warning, \"%s\" accounts usage: %d/%d (invalid) - user dropped",
                uitem.regname, (rc&3), (rc&12)>>2);
         userdata_free(&uitem); 
         continue;  // skip User
      }
// swap accounts if need
     if ((rc & 3) == 2 || (rc & 12) == 4) 
     { 
//SAY("swapping accounts\n");
        SWAP(uitem.inet_acc, uitem.intra_acc);
     } 

//SAY("accounts: inet: %d intra: %d\n", uitem.inet_acc, uitem.intra_acc);

// 4. Get user host/mail/port info

//SAY("(gates: %d) looking for gates\n", cnt_gates);

     for (i=0; i < cnt_gates; i++)
     {  if ( (uitem.inet_acc > 0 && itm_gates[i].accno == uitem.inet_acc)  ||
             (uitem.intra_acc > 0 && itm_gates[i].accno == uitem.intra_acc)   )
        {
//SAY("found inet/mail gate %s\n", itm_gates[i].str);
           // cut gate
           rc = da_rm(&cnt_gates, &itm_gates, sizeof(*itm_gates), i, &item);
           if (rc < 0)
           {  syslog(LOG_ERR, "USERLIST::load_list(da_rm(feature gate)): Error");
              // drop user
              userdata_free(&uitem);
              // drop gate list
              gatelist_free(&cnt_gates, &itm_gates);
              // free UserList
              free_list();
              return (-1);
           }
           i--;  // decrease index
           switch(item.res_id)
           {  case 0: // inet
                 ptr = item.str;
                 str = next_token(&ptr, "/");  // get Inet addr
                 if (str == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): "
                           "invalid inet gate for \"%s\" - ignoring",
                           uitem.regname);
                    break; 
                 }  
                 rc = inet_aton(str, (in_addr*)&(hitem.addr));
                 if (rc != 1)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                           "invalid inet addr for \"%s\" - ignoring gate",
                           uitem.regname);
                    break; 
                 }
                 str = next_token(&ptr, "/");  // get Mask
                 if (str == NULL)
                    hitem.mask = 32;
                 else
                 {  hitem.mask = strtol(str, NULL, 10);
                    if (hitem.mask < 1 || hitem.mask > 32) hitem.mask = 32;
                 }

                 free(item.str);
                 item.str = NULL;  // to avoid free()

                 rc = da_ins(&(uitem.cnt_hosts), &(uitem.itm_hosts), sizeof(hitem), -1, &(hitem));
                 if (rc < 0)  
                 {  syslog(LOG_ERR, "USERLIST::load_list(da_ins(host)): Error");
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }
//SAY("attached (inet).\n");
                 break; 
              case 1: // mail
                 ptr = item.str; 
                 str = next_token(&ptr, "@");  // get login name
                 if (str == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                           "invalid mail gate for \"%s\" - ignoring",
                           uitem.regname);
                    break; 
                 }  
                 mitem.login = stralloc(str);
                 if (mitem.login == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(calloc(mail login)): %m");
                    // free gate string
                    free(item.str);
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }

                 str = next_token(&ptr, "@");  // get domain name
                 if (str == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                           "missing mail domain for \"%s\" - ignoring gate",
                           uitem.regname);
                    free(mitem.login);
                    mitem.login = NULL;
                    break;
                 }
                 mitem.domain = stralloc(str);
                 if (mitem.domain == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(calloc(mail domain)): %m");
                    // free login name
                    free(mitem.login); 
                    // free gate string
                    free(item.str);
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }

                 free(item.str);
                 item.str = NULL;  // to avoid free()

                 rc = da_ins(&(uitem.cnt_mail), &(uitem.itm_mail), sizeof(mitem), -1, &(mitem));
                 if (rc < 0)  
                 {  syslog(LOG_ERR, "USERLIST::load_list(da_ins(mailbox)): Error");
                    // free login name
                    free(mitem.login); 
                    // free domain name
                    free(mitem.domain); 
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }
//SAY("attached (mail).\n");
                 break;
              case 3: // adder
                 ptr = item.str; 
                 str = next_token(&ptr, ":");  // get switch id
                 if (str == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                           "invalid intra gate for \"%s\" - ignoring",
                           uitem.regname);
                    break; 
                 }
                 pitem.switch_id = stralloc(str);
                 if (pitem.switch_id == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(calloc(switch id)): %m");
                    // free gate string
                    free(item.str);
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }

                 str = next_token(&ptr, ":");  // separate port number
                 if (str == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                           "missing port number for \"%s\" - ignoring gate",
                           uitem.regname);
                    free(pitem.switch_id);
                    pitem.switch_id = NULL;
                    break;
                 }
                 pitem.port = strtol(str, NULL, 10);
                 if (pitem.port < 1)
                 {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                           "invalid port number for \"%s\" - ignoring gate",
                           uitem.regname);
                    free(pitem.switch_id);
                    pitem.switch_id = NULL;
                    break;
                 }

                 free(item.str);
                 item.str = NULL;   // to avoid free()

                 rc = da_ins(&(uitem.cnt_ports), &(uitem.itm_ports), sizeof(pitem), -1, &(pitem));
                 if (rc < 0)  
                 {  syslog(LOG_ERR, "USERLIST::load_list(da_ins(port)): Error");
                    // free switch id
                    free(pitem.switch_id); 
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }

//SAY("attached (intra).\n");
                 break;

                 // Other gate types are ignored   

           } // switch res_id

         // clean up
           if (item.str != NULL) 
           {  free(item.str);
              item.str = NULL;
           }
        }  // if 
     }  // for
   // Drop user if no valid gates found
     if (uitem.cnt_mail  == 0 && 
         uitem.cnt_hosts == 0 && 
         uitem.cnt_ports == 0)
      {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
                "no valid gates for \"%s\" - user dropped",
                uitem.regname);
         // drop user
         userdata_free(&uitem);
         continue;
      } 

// 6. Attach Userlist item
      
      rc = da_ins(&(cnt_users), &(itm_users), sizeof(uitem), -1, &(uitem));
      if (rc < 0)  
      {  syslog(LOG_ERR, "USERLIST::load_list(calloc(user)): %m");
         // drop user
         userdata_free(&uitem);
         // drop gate list
         gatelist_free(&cnt_gates, &itm_gates);
         // drop UserList
         free_list();
         return (-1);
      }

//SAY("user attached\n");

   } // while(1)

// 7. Free gates (if any left)


   if (cnt_gates > 0)
   {  syslog(LOG_ERR, "USERLIST::load_list(): Warning, "
             "%d gates left (ignoring)", cnt_gates);

//SAY("unlinked gates left: %d\n", cnt_gates);
//for (i=0; i<cnt_gates; i++)
//  SAY("gate id:%d, acc:%d, str:\"%s\"\n",  itm_gates[i].res_id, itm_gates[i].accno, itm_gates[i].str);

      // destroy gate list
      gatelist_free(&cnt_gates, &itm_gates); 
   }

   return 0;
}


int   USERLIST::user_str (char * buf, int len, int index)
{  int p = 0;


   if (buf == NULL || len < 2 || index > cnt_users)
      return (-1);

   if (index < 0)  // Negative index - print topics
   {  p += snprintf(buf+p, len-p, "  %-10s  %15s      %4s   #", 
                    "имя", "адрес", "хаб");
      return p;
   }

// username
   p += snprintf(buf+p, len-p, "%-12s  ", itm_users[index].regname);
// inet address
   if (itm_users[index].cnt_hosts > 0)   
   {  p += snprintf(buf+p, len-p, "%15s /%2d  ", 
        inet_ntoa(*((in_addr*)(&(itm_users[index].itm_hosts[0].addr)))),
        itm_users[index].itm_hosts[0].mask);
   }
   else
      p += snprintf(buf+p, len-p, "%15s      ", "-");

// Switch/port
   if (itm_users[index].cnt_ports > 0)   
   {  p += snprintf(buf+p, len-p, "%4s  %2d", 
            itm_users[index].itm_ports[0].switch_id,
            itm_users[index].itm_ports[0].port);
   }
   else
      p += snprintf(buf+p, len-p, "%4s   -", "-");

   return p;
}

/*
   Update levels

   >> none
   >> rewrite lighting (two lines)
   >> rewrite all dynamic parts

   >> rewrite all (external)

*/

void  USERLIST::initview()
{
   flags       = ULF_REFRESH;
   first       = 0;
   marked      = 0;
   last_marked = 0;
}



void  USERLIST::refresh()
{  WINOUT   ww;
   int      i;
   char     fmt[8];

   if ((flags & ULF_REFRESH) != 0)
   {  ww.New(this);
      ww.lin  -= 1;
      ww.col  -= 1;
      ww.lins += 2;
      ww.cols += 2;
      Attr(7, 0);
      ww.fill(FT_SINGLE);
      Gotoxy(lin-2, col);
      user_str(listbuf, sizeof(listbuf), (-1));
      uprintf(" %s", listbuf);
      flags = ULF_WINDMOV;
   }

   if ((flags & ULF_WINDMOV) != 0)
   {  
      sprintf(fmt, " %%-%ds", cols-1);

      for (i=0; i<lins; i++)
      {  Gotoxy(lin+i, col);
         if (first+i == marked) Attr(0, 7);
         else Attr(7, 0);

         if (first+i < cnt_users)
            user_str(listbuf, sizeof(listbuf), first+i);
         else
            *listbuf = '\0';

         uprintf(fmt, listbuf);
      }
      flags = 0;
   }

   if ((flags & ULF_LIGHTMOV) != 0)
   {  flags = 0;

      if (last_marked == marked) return;

      sprintf(fmt, " %%-%ds", cols-1);

      if (last_marked >= first        &&
          last_marked < first + lins  &&
          last_marked >= 0            &&
          last_marked < cnt_users)
      {  Attr(7, 0);
         Gotoxy(lin + last_marked - first, col);
         user_str(listbuf, sizeof(listbuf), last_marked);
         uprintf(fmt, listbuf);                
      }

      if (marked >= first        &&
          marked < first + lins  &&
          marked >= 0            &&
          marked < cnt_users)
      {  Attr(0, 7);
         Gotoxy(lin + marked - first, col);
         user_str(listbuf, sizeof(listbuf), marked);
         uprintf(fmt, listbuf);                
      }
   }
}

int cmp_regname (void * user1, void * user2)
{
   return strcasecmp(((userdata_t*)user1)->regname, 
                     ((userdata_t*)user2)->regname) > 0;   
}

int cmp_ip (void * user1, void * user2)
{  
   long   addr1 = 0;
   long   addr2 = 0;

   if (((userdata_t*)user1)->cnt_hosts > 0) 
      addr1 = swap32((long)(((userdata_t*)user1)->itm_hosts->addr));

   if (((userdata_t*)user2)->cnt_hosts > 0) 
      addr2 = swap32((long)(((userdata_t*)user2)->itm_hosts->addr));

   return addr1 > addr2;
}

int cmp_port (void * user1, void * user2)
{
   char * sw1   = "";
   char * sw2   = "";
   int    port1 = 0;
   int    port2 = 0;
   int    rc;

   if (((userdata_t*)user1)->cnt_ports > 0)
   {  sw1   = ((userdata_t*)user1)->itm_ports->switch_id;
      port1 = ((userdata_t*)user1)->itm_ports->port;
   }

   if (((userdata_t*)user2)->cnt_ports > 0)
   {  sw2   = ((userdata_t*)user2)->itm_ports->switch_id;
      port2 = ((userdata_t*)user2)->itm_ports->port;
   }

   rc = strcasecmp(sw1, sw2);
   if (rc != 0) return rc > 0;

   return port1 > port2;   
}

void  USERLIST::sort_regname()
{
   da_bsort(&cnt_users, &itm_users, sizeof(userdata_t), cmp_regname);
}

void  USERLIST::sort_ip()
{
   da_bsort(&cnt_users, &itm_users, sizeof(userdata_t), cmp_ip);
}

void  USERLIST::sort_port()
{
   da_bsort(&cnt_users, &itm_users, sizeof(userdata_t), cmp_port);
}

void  USERLIST::rev_order()
{
//   Gotoxy(1,0); Puts("called"); refresh();
   da_reverse(&cnt_users, &itm_users, sizeof(userdata_t));
}

//
// find & set cursor to first matching login (or do nothing)
//

void  USERLIST::find_login()
{  int tlen;
   int nlen;
   int i;

   tlen = strlen(LoginBuf);
   if (tlen < 1) return;

   for (i=0; i<cnt_users; i++)
   {  nlen = strlen(itm_users[i].regname);
      if (nlen < tlen) continue;
      if (memcmp(itm_users[i].regname, LoginBuf, tlen) == 0) break;
   }

   if (i < cnt_users) 
   {  last_marked = marked;
      marked      = i;
      flags |= ULF_LIGHTMOV;
      if (marked < first)
      {  first = marked;
         flags |= ULF_WINDMOV;
      }
      if (marked >= (first + lins))
      {  first = marked - lins + 1;
         flags |= ULF_WINDMOV;
      }
   }
}
