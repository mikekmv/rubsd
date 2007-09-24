/* $oganer: netio.c,v 1.3 2003/08/05 07:30:00 tm Exp $ */

/*
 * Copyright (c) 2003 Ilya Kovalenko <shadow@oganer.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#include <netio.h> 

void child_handler(int sig);
struct sigaction ignore = {{SIG_IGN}, 0, 0};
struct sigaction handle = {{child_handler}, 0, 0};

/* * * * * * * * * * * * * *\
 *  Global initialization  *
\* * * * * * * * * * * * * */

int  tcp_init(int flags)
{  
   sigaction(SIGPIPE, &ignore, NULL);
   if (flags == SF_ZOMBIES) sigaction(SIGCHLD, &handle, NULL);
   return SUCCESS;
}

/* * * * * * * * * * * * * *\
 *     Open TCP socket     *
\* * * * * * * * * * * * * */

int  tcp_socket(int flags)
{  int sd;
   int flg;
   int rc;

   sd=socket(AF_INET, SOCK_STREAM, 0);
   if (sd < 0)
   {  syslog(LOG_ERR, "tcp_socket(socket): %m");
      return (-1);
   }
   if (flags & SF_NONBLOCK)
   {  flg=fcntl(sd, F_GETFL);
      if (flags == -1)
      {  syslog(LOG_ERR, "tcp_socket(fcntl(F_GETFL)): %m");
         close(sd);
         return (-1);
      }
      flg |= O_NONBLOCK;
      if (fcntl(sd, F_SETFL, flg) == -1)
      {  syslog(LOG_ERR, "tcp_socket(fcntl(F_SETFL)): %m");
         close(sd);
         return (-1);
      }
   } 
   flg=1; // one for setsockopt
   rc=setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &flg, sizeof(flags));
   if (rc==-1)
   {  syslog(LOG_ERR, "tcp_socket(setsockopt(REUSEADDR)): %m");
      close(sd);
      return (-1);
   }   
   rc = setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &flg, sizeof(flags));
   if (rc==-1)
   {  syslog(LOG_ERR, "tcp_socket(setsockopt(KEEPALIVE)): %m");
      close(sd);
      return (-1);
   }   

   return sd;
}

/* * * * * * * * * * * * * * * * *\
 *   Start TCP server on socket  *
\* * * * * * * * * * * * * * * * */

int  tcp_server(int sd, int port, int clients, struct sockaddr_in * name)
{  
   struct sockaddr_in *pname;
   struct sockaddr_in lname;
   socklen_t  namelen;

   pname = name == NULL ? &lname : name;
	
   namelen = (socklen_t)sizeof(struct sockaddr_in);
   
   if (getsockname(sd, (struct sockaddr*)pname, &namelen) != 0)
   {  syslog(LOG_ERR, "tcp_server(getsockname): %m");
      return (-1);
   }
   pname->sin_port=htons(port);
   pname->sin_addr.s_addr=htonl(INADDR_ANY);
   if (bind(sd, (struct sockaddr*)pname, namelen)!=0)
   {  syslog(LOG_ERR, "tcp_server(bind): %m");
      return (-1);
   }
   if (listen(sd, clients) != 0)
   {  syslog(LOG_ERR, "tcp_server(listen): %m");
      return (-1);
   }
   return (sd);
}

/* * * * * * * * * * * * * * * * * * * * *\
 *  Accept (or check) server connection  *
\* * * * * * * * * * * * * * * * * * * * */

int tcp_accept(int sd)
{  int  sdnew;
   struct sockaddr_in   peer;
   socklen_t            namelen;

   sdnew = accept(sd, (struct sockaddr*)&peer, &namelen);
   if (sdnew == -1)
   {  if (errno == EWOULDBLOCK) sdnew=NO_CLIENTS;
      else if (errno != EINTR) syslog(LOG_ERR, "server_accept(): %m");
   }

   return sdnew;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *  Fork client process (closing parent/child socket)  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * */

int tcp_forkclient(int sd, int clsd)
{  pid_t rc;

   rc = fork();
   switch(rc)
   {  case 0:   // on child
         close(sd);
         break;
      case -1:  // on error
         syslog(LOG_ERR, "tcp_server(fork): %m");
      default:  // on parent
         close(clsd);
   }
   return (int) rc;  
}

/* * * * * * * * * * * * * * * * * * * * * *\
 *  Start TCP client (connect to server)   *
\* * * * * * * * * * * * * * * * * * * * * */

int tcp_client(int sd, char * host, int port)
{  struct sockaddr_in name;
   socklen_t          namelen=(socklen_t)sizeof(name);
  
   if (getsockname(sd, (struct sockaddr*)&name, &namelen) != 0)
   {  syslog(LOG_ERR, "tcp_client(getsockname): %m");
      return (-1);
   }
   name.sin_port=htons(port);
   inet_aton(host, (struct in_addr*)&(name.sin_addr));
   if (connect(sd, (struct sockaddr*)&name, namelen) != 0)
   {  syslog(LOG_ERR, "tcp_client(connect): %m");
      return (-1);
   }

   return SUCCESS;
}

void child_handler(int sig)
{  int s_errno=errno;  

   while (wait3(NULL, WNOHANG, NULL) > 0);
   errno=s_errno;
}
