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

#include "da.h"
#include "global.h"

#define SAY  printf


char * AccListFile = "/var/bee/payman.lst";
char * LinksFile   = "/var/bee/reslinks.dat";
char * linklock    = "/var/bee/link.lock";

char   rlnk_delim[]= " \t\n\t";

USERLIST UserList = {0,0,0,0, 0,0,0, 0,NULL, 0,NULL};

/* Get next token (skip multy delimiters) */

char * next_token(char ** ptr, char * delim)
{  char * str;

   do 
   {  str = strsep(ptr, delim);
      if (str == NULL) break;
   } while (*str == '\0');

   return str;
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
   {  syslog(LOG_ERR, "USERLIST::load_accs(open): %m");
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

// Obtain shared lock to links file
   lockfd = open(linklock, O_CREAT);
   if (lockfd == -1)
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
// resource id (skip)
      str = next_token(&ptr, rlnk_delim);
      if (str == NULL) continue;  // skip invalid line
      // (skip gate ID)
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
      item.str = (char *) calloc(1, strlen(str)+1); // plus '\0'
      if (item.str == NULL)
      {  syslog(LOG_ERR, "USERLIST::load_list(calloc): %m");
         da_empty(&cnt_gates, &itm_gates, sizeof(*itm_gates));
         fclose(f);
         close(lockfd);
         return (-1);
      }
      strcpy(item.str, str);

// add gate to list
      rc = da_ins(&cnt_gates, &itm_gates, sizeof(item), (-1), &item);
      if (rc < 0)
      {  syslog(LOG_ERR, "USERLIST::load_list(da_ins): Error");
         free(item.str);
         gatelist_free(&cnt_gates, &itm_gates);
         fclose(f);
         close(lockfd);
         return (-1);
      }
      item.str = NULL; // to avoid double free()
   }

// close & unlock
   fclose(f);
   close(lockfd);
   
// Lookup Users
   while(1)
   {  
SAY("----- LOOP -----\n");
// 1. Get any adder gate (user name, first account)
SAY("(gates: %d) looking for any adder\n", cnt_gates);
   // find
      for (i=0; i < cnt_gates; i++)
         if (itm_gates[i].res_id == 2) break; 
      if (i >= cnt_gates)     // i.e. no adder gates left, quit
      {    
         SAY("not found - JOB DONE\n");
         break;  // i.e. no adder gates left, quit
      }
SAY("found \"%s\" #%d, cutting out\n", itm_gates[i].str, itm_gates[i].accno);
   // cut gate
      rc = da_rm(&cnt_gates, &itm_gates, sizeof(*itm_gates), i, &item);
      if (rc < 0)
      {  
SAY("****** SYSTEM ERROR - ABORT\n");
         syslog(LOG_ERR, "USERLIST::load_list(da_rm(any adder)): Error");
         // drop gate list
         gatelist_free(&cnt_gates, &itm_gates);
         // free UserList
         free_list();
         return (-1);
      }
   // get initial userlist item data from gate
      memset(&uitem, 0, sizeof(uitem));
      uitem.intra_acc = (-1); // i.e. second accout is n/a by default

      uitem.inet_acc = item.accno;
      uitem.regname  = item.str;

      item.str = NULL; // to avoid double free()                  

// 2. Find & get same named adder gates.
//    If more then 1 gate found - drop it & drop user
SAY("(gates: %d) looking for \"%s\" adder\n", cnt_gates, uitem.regname);
      for (i=0; i < cnt_gates; i++)
      {  if (itm_gates[i].res_id == 2 &&
             strcmp(itm_gates[i].str, uitem.regname) == 0)
         {  
SAY("found \"%s\" #%d, cutting out\n", itm_gates[i].str, itm_gates[i].accno);
            rc = da_rm(&cnt_gates, &itm_gates, sizeof(*itm_gates), i, &item);
            if (rc < 0)
            {  
SAY("****** SYSTEM ERROR - ABORT\n");
               syslog(LOG_ERR, "USERLIST::load_list(da_rm(any adder)): Error");
               // drop gate list
               gatelist_free(&cnt_gates, &itm_gates);
               // free UserList
               free_list();
               return (-1);
            }

            if (uitem.intra_acc == (-1))
            {  
SAY("storing second account\n");
               uitem.intra_acc = item.accno;
            }
            else
            {  
SAY("extra account ! drop accounts\n");
               if (uitem.intra_acc != (-2))
               {  
SAY("set drop flag\n");
                  uitem.intra_acc = (-2);    // extra adder gates
               }
            }
            free(item.str);  // free string
            item.str = NULL; // to avoid double free()
         }
      }
      if (uitem.intra_acc == (-2))
      {  
SAY("drop flag set - drop user\n");
         free(uitem.regname);
         uitem.regname = NULL;
         continue;  // skip User
      }

// 3. Determine which account are inet and which are intra
//     inet/mail gates (0/1) = inet
//     intera gate     (3)   = intra
//     no gates              = undefined

      rc = 0;

SAY("determine accounts usage\n");
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

SAY("USAGE: acc1(#%d): %d acc2(#%d):%d\n", uitem.inet_acc, (rc&3), uitem.intra_acc, (rc&12)>>2);

// drop invalid cases
      if (rc == 0 || (rc & 3) == 3 || (rc & 12) == 12 || (rc & 3) == ((rc & 12)>>2))
      {  free(uitem.regname);
         uitem.regname = NULL;
         continue;  // skip User
      }
// swap accounts if need
     if ((rc & 3) == 2 || (rc & 12) == 4) 
     { 
SAY("swapping accounts\n");
        rc              = uitem.intra_acc;
        uitem.intra_acc = uitem.inet_acc;
        uitem.inet_acc  = rc;

//      SWAP(uitem.inet_acc, uitem.intra_acc);
     } 

SAY("accounts: inet: %d intra: %d\n", uitem.inet_acc, uitem.intra_acc);

SAY("(gates: %d) looking for gates\n", cnt_gates);
// 4. Get host/mail/port info
     for (i=0; i < cnt_gates; i++)
     {  if (uitem.inet_acc > 0 && itm_gates[i].accno == uitem.inet_acc)
        {
SAY("found inet/mail gate %s\n", itm_gates[i].str);
           // cut gate
           rc = da_rm(&cnt_gates, &itm_gates, sizeof(*itm_gates), i, &item);
           if (rc < 0)
           {  syslog(LOG_ERR, "USERLIST::load_list(da_rm(any adder)): Error");
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
                 str = next_token(&ptr, "/");  // separate Inet addr
                 if (str == NULL)
                 {  free(item.str);
                    item.str = NULL;
                    break; 
                 }  
                 rc = inet_aton(str, (in_addr*)&(hitem.addr));
                 if (rc != 1)
                 {  free(item.str);
                    item.str = NULL;
                    break; 
                 }
                 str = next_token(&ptr, "/");  // separate Mask
                 if (str == NULL)
                    hitem.mask = 32;
                 else
                 {  hitem.mask = strtol(str, NULL, 10);
                    if (hitem.mask < 1 || hitem.mask > 32) hitem.mask = 32;
                 }

                 free(item.str);
                 item.str = NULL;  

                 rc = da_ins(&(uitem.cnt_hosts), &(uitem.itm_hosts), sizeof(*(uitem.itm_hosts)), -1, &(hitem));
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
SAY("attached (inet).\n");
                 break; 
              case 1: // mail
                 ptr = item.str; 
                 str = next_token(&ptr, "@");  // separate login name
                 if (str == NULL)
                 {  free(item.str);
                    item.str = NULL;
                    break; 
                 }  
                 mitem.login = (char*) calloc(1, strlen(str)+1);
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
                 strcpy(mitem.login, str);
                 str = next_token(&ptr, "@");  // separate domain name
                 if (str == NULL)
                 {  free(mitem.login);
                    mitem.login = NULL;
                    free(item.str);
                    item.str = NULL;
                    break;
                 }
                 mitem.domain = (char*) calloc(1, strlen(str)+1);
                 if (mitem.domain == NULL)
                 {  syslog(LOG_ERR, "USERLIST::load_list(calloc(mail login)): %m");
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
                 strcpy(mitem.domain, str);

                 free(item.str);
                 item.str = NULL;

                 rc = da_ins(&(uitem.cnt_mail), &(uitem.itm_mail), sizeof(*(uitem.itm_mail)), -1, &(mitem));
                 if (rc < 0)  
                 {  syslog(LOG_ERR, "USERLIST::load_list(da_ins(host)): Error");
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
SAY("attached (mail).\n");
                 break; 
           } // switch res_id
         // clean up (other gates)
           if (item.str != NULL) 
           {  free(item.str);
              item.str = NULL;
           }
        } 

        if (uitem.intra_acc > 0 && itm_gates[i].accno == uitem.intra_acc)
        {
SAY("found intra gate %s\n", itm_gates[i].str);
           // cut gate
           rc = da_rm(&cnt_gates, &itm_gates, sizeof(*itm_gates), i, &item);
           if (rc < 0)
           {  syslog(LOG_ERR, "USERLIST::load_list(da_rm(any adder)): Error");
              // drop user
              userdata_free(&uitem);
              // drop gate list
              gatelist_free(&cnt_gates, &itm_gates);
              // drop UserList
              free_list();
              return (-1);
           }
           i--;  // decrease index
           switch(item.res_id)
           {  case 3: // adder
                 ptr = item.str; 
                 str = next_token(&ptr, ":");  // separate switch id
                 if (str == NULL)
                 {  free(item.str);
                    item.str = NULL;
                    break; 
                 }
                 pitem.switch_id = (char*) calloc(1, strlen(str)+1);
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
                 strcpy(pitem.switch_id, str);
                 str = next_token(&ptr, ":");  // separate port number
                 if (str == NULL)
                 {  free(pitem.switch_id);
                    pitem.switch_id = NULL;
                    free(item.str);
                    item.str = NULL;
                    break;
                 }
                 pitem.port = strtol(str, NULL, 10);
                 if (pitem.port < 1)
                 {  free(pitem.switch_id);
                    pitem.switch_id = NULL;
                    free(item.str);
                    item.str = NULL;
                    break;
                 }

                 free(item.str);
                 item.str = NULL;

                 rc = da_ins(&(uitem.cnt_ports), &(uitem.itm_ports), sizeof(*(uitem.itm_ports)), -1, &(pitem));
                 if (rc < 0)  
                 {  syslog(LOG_ERR, "USERLIST::load_list(da_ins(host)): Error");
                    // free login name
                    free(pitem.switch_id); 
                    // drop user
                    userdata_free(&uitem);
                    // drop gate list
                    gatelist_free(&cnt_gates, &itm_gates);
                    // drop UserList
                    free_list();
                    return (-1);
                 }

SAY("attached (intra).\n");
                 break;
           } // switch
           // clean up (other gates)
           if (item.str != NULL) 
           {  free(item.str);
              item.str = NULL;
           }
        }
     }
   // Drop user if no valid gates found
     if (uitem.cnt_mail  == 0 && 
         uitem.cnt_hosts == 0 && 
         uitem.cnt_ports == 0)
      {  
         // drop user
         userdata_free(&uitem);
         continue;
      } 

// 6. Attach Userlist item
      
      rc = da_ins(&(cnt_users), &(itm_users), sizeof(uitem), -1, &(uitem));
      if (rc < 0)  
      {  syslog(LOG_ERR, "USERLIST::load_list(calloc(switch id)): %m");
         // drop user
         userdata_free(&uitem);
         // drop gate list
         gatelist_free(&cnt_gates, &itm_gates);
         // drop UserList
         free_list();
         return (-1);
      }
SAY("user attached\n");
SAY("accounts: inet: %d intra: %d\n", uitem.inet_acc, uitem.intra_acc);
   } // while(1)

SAY("CLEAN-UP & EXIT\n");

SAY("unlinked gates left: %d\n", cnt_gates);
for (i=0; i<cnt_gates; i++)
  SAY("gate id:%d, acc:%d, str:\"%s\"\n",  itm_gates[i].res_id, itm_gates[i].accno, itm_gates[i].str);


// 7. Free gates (if any left)
   gatelist_free(&cnt_gates, &itm_gates); 

   return 0;
}
