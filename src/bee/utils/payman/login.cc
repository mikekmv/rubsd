#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <bee.h>

#include "global.h"
#include "login.h"
#include "da.h"
#include "log.h"

char *    al_idents[ALEVELS]=
{  "none",   // 0
   "view",   // 1
   "pay",    // 2
   "master", // 3 
   "total"   // 4
};

char    UsersFile[] = "/var/bee/payman.users";
char    usrdelim[]  = " \t\n\r";

int       cnt_logins = 0;
login_t * itm_logins = NULL;

char    loggeduser[16]="\0";

char    lastlogin[16];
char    lastpass[32];

int loginDialogProc(DIALOG * th, int action, ulong param)
{
   if (action == PA_EPILOGUE)
   {  if (param == ID_OK) 
      {  strlcpy(lastlogin,
              (char *)(((COMBOX*)(th->ctrl[2].mem))->buff),
              sizeof(lastlogin) );
         strlcpy(lastpass, 
               (char *)(((COMBOX*)(th->ctrl[3].mem))->buff),
               sizeof(lastpass) );
      }
      return param;
   }

   return RET_CONT;
}

CONTROL loginDialogControls[]=
{  {  1,2,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Логин: \0",
      0,0,0,0,0
   },
   {  3,2,1,8,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"Пароль: \0",
      0,0,0,0,0
   },
   {  1,10,1,16,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_EDIT,
      "\0",
      0,
      8,
      0,
      0,0
   },
   {  3,10,1,16,
      CS_DEFAULT,
      0xAA55,
      ComboBoxControl,
      CBT_EDIT | CBT_PASSWORD,
      "\0",
      0,
      16,
      0,
      0,0
   },
   {  5,1,2,27,
      CS_DISABLED,
      0xAA55,
      GenControl,
      CT_STATIC,
"(пустой логин - блокировка)\0",
      0,0,0,0,0
   },
   {  0,0,0,0,
      0,
      0,
      0,
      0,
      0,
      0,0,0,0,0
   }
};

DIALOG  loginDialog=
{  0,0,
   6,30,
   DS_FRAMED | DS_EPILOGED,
   DA_DEFAULT,
   SI_NEUTRAL,
   "Аутентификация\0",
   loginDialogProc,
   0,
   0,
   0,0,
   loginDialogControls
};

int LogInUser()
{  int rc;

// Get login/pass
   rc = loginDialog.Dialog(0);
// Trap Cancel
   if (rc != ID_OK) return rc;

// Trap empty name (log-out)   
   if (*lastlogin == '\0')
   {  AccessLevel = 0;
      *loggeduser = '\0';
      return ID_OK;
   }

// check name/pass
   rc = login_check(lastlogin, lastpass);

   switch(rc)
   {  
      case 0:
         MessageBox("Доступ запрещен\0",
                     " Данный профиль запрещен \0",
                    MB_OK | MB_NEUTRAL);
         break;
      case (-1):
         MessageBox("Доступ запрещен\0",
                    " Неправильный логин или пароль \0",
                    MB_OK | MB_NEUTRAL);
         break;
      case (-2):
         MessageBox("Доступ запрещен\0",
                    " Произошла системная ошибка \0",
                    MB_OK | MB_NEUTRAL);
         break;
      default:
         if (rc > 0 && rc < ALEVELS) 
         {  AccessLevel = rc;
            strlcpy(loggeduser, lastlogin, sizeof(loggeduser));
            log_write("login %s", loggeduser);
         }
         else syslog(LOG_ERR, "LogInUser(login_check): invalid AL returned");
   }

   return ID_OK;  // hmm
}

char buf[128];

int logins_load()
{  FILE    * f;  
   login_t   item;
   int       rc;
   char    * ptr;
   char    * str;
   int       i;

// Open links file
   f = fopen(UsersFile, "r");
   if (f == NULL)
   {  syslog(LOG_ERR, "load_logins(fopen(%s)): %m", UsersFile);
      return (-1);
   }

// Cleanup item
   memset(&item, 0, sizeof(item));

// Parse cycle
   while(fgets(buf, sizeof(buf), f) != NULL)
   {
// free memory (on invalid lines)
      if (item.name != NULL) 
      {  free(item.name);
         item.name = NULL;
      }
      if (item.passhash != NULL) 
      {  free(item.passhash);
         item.passhash = NULL;
      }

// skip comment
      if (*buf == '#') continue;
      ptr = buf;
// get login name
      str = next_token(&ptr, usrdelim);
      if (str == NULL) continue;  // skip empty line
      item.name = stralloc(str);
      if (item.name == NULL)
      {  syslog(LOG_ERR, "load_logins(strnalloc): NULL return");
         fclose(f);
         da_empty(&cnt_logins, &itm_logins, sizeof(login_t));  
         return (-1);
      }
// get access name
      str = next_token(&ptr, usrdelim);
      if (str == NULL) 
      {  syslog(LOG_ERR, "load_logins(): Warning:"
                "invalid record for \"%s\" (skipped)", item.name);
         continue;
      }
// tranlate access name -> level number
      for (i = 0; i < ALEVELS; i++)
         if (strcmp(al_idents[i], str) == 0) break;
      if (i >= ALEVELS) continue;        // skip invalid line
      item.level = i;
// get password hash
      str = next_token(&ptr, usrdelim);
      if (str == NULL) continue;  // skip empty line
      item.passhash = stralloc(str);
      if (item.name == NULL)
      {  syslog(LOG_ERR, "load_logins(strnalloc): NULL return");
         free(item.name);
         fclose(f);
         da_empty(&cnt_logins, &itm_logins, sizeof(login_t));
         return (-1);
      }
// Add item to array
      rc = da_ins(&cnt_logins, &itm_logins, sizeof(login_t), (-1), &item);
      if (rc < 0)
      {  syslog(LOG_ERR, "load_logins(da_ins): Error");
         free(item.name);
         free(item.passhash);
         fclose(f);
         da_empty(&cnt_logins, &itm_logins, sizeof(login_t));
         return (-1);
      } 

// Success: NULL pointers (to avoid free())
      item.name     = NULL;
      item.passhash = NULL;      
   } 

// free memory (on invalid lines)
   if (item.name != NULL) 
   {  free(item.name);
      item.name = NULL;
   }
   if (item.passhash != NULL) 
   {  free(item.passhash);
      item.passhash = NULL;
   }
   
   return 0;   
}

int logins_destroy()
{
   da_empty(&cnt_logins, &itm_logins, sizeof(login_t));
   return 0;
}

int login_check   (char * login, char * pass)
{  char * ptr;
   int    i;
   
// Find login name
   for (i=0; i < cnt_logins; i++)
      if (strcasecmp(login, itm_logins[i].name) == 0) break;

// Not found trap
   if (i >= cnt_logins) return (-1);  // Negtive answer

// Encrypt password
   ptr = crypt(pass, itm_logins[i].passhash);
   if (ptr == NULL)
   {  syslog(LOG_ERR, "login_check(crypt): %m (NULL returned)");
      return (-2); // Error
   } 

// Check two hashes
   if (strcmp(ptr, itm_logins[i].passhash) != 0) return (-1);  // Negative answer

   return itm_logins[i].level;
}
