/* $Bee$ */
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

void child_handler(int sig);

struct sigaction ignore = {{SIG_IGN}, 0, 0};
struct sigaction handle = {{child_handler}, 0, 0};

int  tcp_server(int service, int local)
{  int rc;
   int sd;
   struct sockaddr_in name;
   struct sockaddr_in peer;
   socklen_t          namelen=(socklen_t)sizeof(name);
   int sdnew;
   
   sigaction(SIGPIPE, &ignore, NULL);
   sigaction(SIGCHLD, &handle, NULL);
   sd=socket(AF_INET, SOCK_STREAM, 0);
   if (sd < 0)
   {  syslog(LOG_ERR, "tcp_server(socket): %m");
      return (-1);
   }
   if (getsockname(sd, (struct sockaddr*)&name, &namelen) != 0)
   {  syslog(LOG_ERR, "tcp_server(getsockname): %m");
      close(sd);
      return (-1);
   }
   name.sin_port=htons(service);
   name.sin_addr.s_addr=htonl(INADDR_ANY);
   if (bind(sd, (struct sockaddr*)&name, namelen)!=0)
   {  syslog(LOG_ERR, "tcp_server(bind): %m");
      close (sd);
      return (-1);
   }
   while(1)
   {  
      if (listen(sd, 8) != 0)
      {  syslog(LOG_ERR, "tcp_server(listen): %m");
         close(sd);   
         return (-1);
      }
      do      
      {  sdnew=accept(sd, (struct sockaddr*)&peer, &namelen);
      }  while(sdnew == (-1) && errno == EINTR);
      if (peer.sin_addr.s_addr != inet_addr("127.0.0.1") && local)
      {  syslog(LOG_ERR, "S down socket");
         shutdown(sdnew, SHUT_RDWR);
         close(sdnew);
         continue;
      }
      if (sdnew < 0) break;
      rc=fork();
      if (rc==0) 
      {  
         break;  // on child
      }
      if (rc==(-1))
      {  syslog(LOG_ERR, "tcp_server(fork): %m");
         shutdown(sdnew, SHUT_RDWR);
      }
      close(sdnew);
   }
   close(sd);
   return sdnew;
}

int tcp_client(char * host, int service)
{ 
   int sd;
   struct sockaddr_in name;
   struct sockaddr_in peer;
   socklen_t          namelen=(socklen_t)sizeof(name);
  
   sigaction(SIGPIPE, &ignore, NULL);
   sd=socket(AF_INET, SOCK_STREAM, 0);
   if (sd < 0)
   {  syslog(LOG_ERR, "tcp_client(socket): %m");
      return (-1);
   }
   if (getsockname(sd, (struct sockaddr*)&name, &namelen) != 0)
   {  syslog(LOG_ERR, "tcp_client(getsockname): %m");
      close(sd);
      return (-1);
   }
   peer=name;
   peer.sin_port=htons(service);
   inet_aton(host, (struct in_addr*)&(peer.sin_addr));
   if (connect(sd, (struct sockaddr*)&peer, namelen) != 0)
   {  syslog(LOG_ERR, "tcp_client(connect): %m");
      close (sd);
      return (-1);
   }
   if (getpeername(sd, (struct sockaddr*)&peer, &namelen) != 0)
   {  syslog(LOG_ERR, "tcp_client(getsockname): %m");
      close(sd);
      return (-1);
   }
   return sd;
}

int link_request(link_t * ld, char * host, int service)
{  int rc;

   rc=tcp_client(host, service);
   if (rc == (-1)) return (-1);
   ld->fd=rc;
   return 0;
}

int link_wait(link_t * ld, int service)
{  int rc;

   if (ld->fStdio) return 0;
   rc=tcp_server(service, ld->Local);
   if (rc == (-1)) return (-1);
   ld->fd=rc;
   return 0;
}

int link_close(link_t * ld)
{  if (ld->fStdio) return 0;
   shutdown(ld->fd, SHUT_RDWR);
   close(ld->fd);
   return 0;   
}

int link_chkin(link_t * ld)
{  int bytes;
   int rc;

   if (ld->fStdio) return 0;
   rc=ioctl(ld->fd, FIONREAD, &bytes);
   return rc>=0 ? bytes>0 : rc;
}

int link_gets(link_t * ld, char * buf, int len)
{  char   c;
   int    rc;
   int    n=0;
   char * ptr=buf; 

   do
   {  if (ld->fStdio)
      {  rc=getchar();
         c=(char)rc;
         if (rc==-1) return LINK_DOWN;
      }
      else
      {  rc=recv(ld->fd, &c, 1, 0);
         if (rc==-1) return LINK_ERROR;
         if (rc==0) return LINK_DOWN;
      }
      if (c=='\r' || c=='\n' || n>=len-1) continue;
      *(ptr++)=c;
      n++; 
   } while(c != '\n');
   if (len != 0) *ptr='\0';
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
    rc=send(ld->fd, buf, strlen(buf), 0);
    if (rc >= strlen(buf)) rc=send(ld->fd, "\r\n", 2, 0);
    else if (rc >= 0) return LINK_DOWN;
    if (rc==-1) return LINK_ERROR;
    if (rc < 2) return LINK_DOWN;
    return SUCCESS;
}

void child_handler(int sig)
{  int s_errno=errno;  

   while (wait3(NULL, WNOHANG, NULL) > 0);
   errno=s_errno;
}

 /* 
   Wait for billing answer (000 or errno) ignore other values
   return errno && Error string 
 */

int answait(link_t * ld, int event, char * buf, int sz, char ** msg)
{  char        * ptr;
   char        * str;
   int           rc;
   int           errno=(-1);

   do
   {  if ((rc=link_gets(ld, buf, sz))<0) return rc;
      ptr=buf;
      str=next_token(&ptr, " \n\t");
      if (str == NULL) continue;
      errno=strtol(str, NULL, 0);
   } while (errno != 0 && errno < 400 && errno != event);
   *msg=ptr;
   return errno;
}


