#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>

#include <bee.h>
#include <ipc.h>

#include "beetalk.h"


link_t  link;

char       * host = BEE_ADDR;
int          port = BEE_SERVICE;

char         linbuf[128];
char       * msg = NULL;

int bee_enter()
{  int     rc;

   rc = link_request(&link, host, port);
   if (rc < 0)
   {  syslog(LOG_ERR, "bee_enter(link_request): Error");
      return (-1);
   }

   rc = answait(&link, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN)  syslog(LOG_ERR, "bee_enter(hello wait): Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "bee_enter(hello wait): %m");
      if (rc >= 400)        syslog(LOG_ERR, "bee_enter(hello wait): %03d %s", rc, msg);
      link_close(&link);
      return (-1);
   }

   link_puts(&link, "machine");
   rc = answait(&link, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN)  syslog(LOG_ERR, "bee_enter(machine): Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "bee_enter(machine): %m");
      if (rc >= 400)        syslog(LOG_ERR, "bee_enter(machine): %03d %s", rc, msg);
      link_close(&link);
      return (-1);
   }

   return 0;
}

int bee_leave()
{
   link_puts(&link, "exit");
   link_close(&link);
   return 0;
}

int bee_send    (const char * cmd, const char * argfmt, ...)
{
   va_list   ap;
   char    * buf = NULL;

   if (cmd == NULL) return (-1);

   if (argfmt != NULL)
   {  va_start(ap, argfmt);
      vasprintf(&buf, argfmt, ap);
      va_end(ap);
      if (buf == NULL)
      {  syslog(LOG_ERR, "bee_send(vasprintf): Error");
         return (-1);
      }
   }

   link_puts(&link, "%s%s%s", cmd, buf == NULL ? "" : " ",
                                   buf == NULL ? "" : buf);

   if (buf != NULL) free(buf);
    
   return 0;
}

int bee_recv    (int code, char ** pmsg, int * last)
{  int rc;

   rc = answait(&link, code, linbuf, sizeof(linbuf), pmsg);
   if (last != NULL)
   {  if (rc == 0 || rc >= 400) *last = 1;
      else *last = 0;
   } 

   return rc; 
}

int bee_update()
{  int rc;

   link_puts(&link, "update");
   rc = answait(&link, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN)  syslog(LOG_ERR, "bee_update(): Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "bee_update(): %m");
      if (rc >= 400)        syslog(LOG_ERR, "bee_update(): %03d %s", rc, msg);
      return (-1);
   }

   return 0;
}

int bee_intraupdate()
{  int rc;

   link_puts(&link, "intraupdate");
   rc = answait(&link, RET_SUCCESS, linbuf, sizeof(linbuf), &msg);
   if (rc != RET_SUCCESS)
   {  if (rc == LINK_DOWN)  syslog(LOG_ERR, "bee_intraupdate(): Unexpected link down");
      if (rc == LINK_ERROR) syslog(LOG_ERR, "bee_intraupdate(): %m");
      if (rc >= 400)        syslog(LOG_ERR, "bee_intraupdate(): %03d %s", rc, msg);
      return (-1);
   }

   return 0;
}

