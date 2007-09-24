/* $RuOBSD: ipc.c,v 1.7 2004/09/11 14:22:04 shadow Exp $ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <bee.h>
#include <ipc.h> 

int link_request(link_t * ld, char * host, int service)
{  int rc;

   rc = tcp_init(0);
   if (rc < 0) return (-1);

   rc = tcp_socket(0);
   if (rc < 0) return (-1);
   ld->fd = rc;
   
   rc = tcp_client(ld->fd, host, service);
   if (rc == (-1)) return (-1);

   return 0;
}

int link_wait(link_t * ld, int service)
{  int   rc;
   int   sdnew;

   if (ld->fStdio) return 0;

   rc = tcp_init(SF_ZOMBIES);
   if (rc < 0) return (-1);

   rc = tcp_socket(0);
   if (rc < 0) return (-1);
   ld->fd = rc;

   rc = tcp_server(ld->fd, service, 8, NULL);
   if (rc < 0) return (-1);


   while(1)
   {  do
      {  sdnew = tcp_accept(ld->fd);
      }  while(sdnew == (-1) && errno == EINTR);
      if (sdnew < 0) break;

      rc = tcp_forkclient(ld->fd, sdnew);
      if (rc == 0)   // child
      {  ld->fd = sdnew;
         break;
      } 
   } 

   return 0;
}

int link_close(link_t * ld)
{
   if (ld->fStdio) return 0;

   shutdown(ld->fd, SHUT_RDWR);
   close(ld->fd);

   return 0;   
}

int link_chkin(link_t * ld)
{  int    bytes;
   int    rc;

   if (ld->fStdio) return 0;

   rc = ioctl(ld->fd, FIONREAD, &bytes);

   return rc >= 0 ? bytes > 0 : rc;
}

int link_gets(link_t * ld, char * buf, int len)
{  char   c;
   int    rc;
   int    n   = 0;
   char * ptr = buf; 

   do
   {  if (ld->fStdio)
      {  rc = getchar();
         c = (char)rc;
         if (rc == -1) return LINK_DOWN;
      }
      else
      {  rc = recv(ld->fd, &c, 1, 0);
         if (rc == -1) return LINK_ERROR;
         if (rc == 0) return LINK_DOWN;
      }
      if (c == '\r' || c == '\n' || n >= len-1) continue;
      *(ptr++) = c;
      n++; 
   } while(c != '\n');

   if (len != 0) *ptr = '\0';
   return 0;
}

int link_puts(link_t * ld, char * str, ...)
{   int     rc;
    va_list ap;
    char    buf[128];

    if (ld->fStdio) 
    {  va_start(ap, str);
       vprintf(str, ap);
       va_end(ap);
       printf("\r\n");
       return 0;
    }

    va_start(ap, str);
    vsnprintf(buf, sizeof(buf), str, ap);
    va_end(ap);

    rc = send(ld->fd, buf, strlen(buf), 0);
    if (rc >= strlen(buf)) rc = send(ld->fd, "\r\n", 2, 0);
    else if (rc >= 0) return LINK_DOWN;

    if (rc == -1) return LINK_ERROR;
    if (rc < 2) return LINK_DOWN;

    return SUCCESS;
}

 /* 
   Wait for billing answer (000 or errno) ignore other values
   return errno && Error string 
 */

int answait(link_t * ld, int event, char * buf, int sz, char ** msg)
{  char        * ptr;
   char        * str;
   int           rc;
   int           xerrno = (-1);

   do
   {  if ((rc = link_gets(ld, buf, sz))<0) return rc;

      ptr = buf;
      str = next_token(&ptr, " \n\t");
      if (str == NULL) continue;

      xerrno = strtol(str, NULL, 10);
   } while (xerrno != 0 && xerrno < 400 && xerrno != event);

   *msg = ptr;

   return xerrno;
}
