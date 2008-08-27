s/* $RuOBSD: prox.c,v 1.5 2003/04/29 16:43:18 tm Exp $ */
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include <ipc.h>

#define SUCCESS 0

/*
  1. Start tcp server
  2. Connect to server
  3. pass data
  4. detect socket close
  5. close sockets
*/

link_t client;     // proxy to client link
link_t server;     // proxy to server link
int    OwnService=49163;
int    Service=80;
char * Address="127.0.0.1";

char * logname="/var/tmp/snif.out"; 
FILE * logfd=NULL;
int    lastpeer=0;      // 0-nobody, 1-client, 2-server

char * buffer[512];

extern char * __progname;
void usage(int code);
void write_log(int peer, void * data, int size);

void proxy(int cfd, int sfd)
{
   int     rc;
   int     i;
   int     nfds;
   fd_set  rd_set;
   fd_set  ex_set;
   int     bytes;

   nfds=(cfd > sfd ? cfd : sfd)+1;

   while(1)
   {  FD_ZERO(&rd_set);
      FD_SET(cfd, &rd_set);
      FD_SET(sfd, &rd_set);
      FD_ZERO(&ex_set);
      FD_SET(sfd, &ex_set);
      FD_SET(cfd, &ex_set);
      rc=select(nfds, &rd_set, NULL, &ex_set, NULL);
      if (rc<0)
      {  syslog(LOG_ERR, "func(select()): %m");
         break;
      }
      if (rc==0)
      {  syslog(LOG_ERR, "func(select()): timeout :)");
         break;
      }
      if (FD_ISSET(sfd, &ex_set))
      {  syslog(LOG_ERR, "server exeption");
         break;
      }
      if (FD_ISSET(cfd, &ex_set))
      {  syslog(LOG_ERR, "client exeption");
         break;
      }
      if (FD_ISSET(cfd, &rd_set))
      {  rc=ioctl(cfd, FIONREAD, &bytes);
         if (rc < 0)
         {  syslog(LOG_ERR, "func(ioctl): %m");
            break;
         }
         if (bytes==0)
         {  syslog(LOG_ERR, "client bytes = 0");
            break;
         }
         for (i=0; i<bytes/sizeof(buffer); i++)
         {  rc=recv(cfd, buffer, sizeof(buffer), 0);
            if (rc < sizeof(buffer))
            {  syslog(LOG_ERR, "func(recv): %s %d(%d), %d %d",
               rc < 0 ? strerror(errno): "(M partial read)",
               rc, sizeof(buffer), bytes, bytes/sizeof(buffer));
            }
            else
            {  write_log(1, buffer, sizeof(buffer));
               rc=send(sfd, buffer, sizeof(buffer), 0);
               if (rc < sizeof(buffer))
               {  syslog(LOG_ERR, "func(send): %s %d(%d)",
                  rc < 0 ? strerror(errno): "(M partial write)",
                  rc, sizeof(buffer));
               }
            }
         }
         bytes %= sizeof(buffer);
         if (bytes > 0)
         {  rc=recv(cfd, buffer, bytes, 0);
            if (rc < bytes)
            {  syslog(LOG_ERR, "func(recv): %s %d (%d)",
               rc < 0 ? strerror(errno): "(S partial read)",
               rc, bytes);
            }
            else
            {  write_log(1, buffer, bytes);
               rc=send(sfd, buffer, bytes, 0);
               if (rc < bytes)
               {  syslog(LOG_ERR, "func(send): %s %d(%d)",
                  rc < 0 ? strerror(errno): "(S partial write)",
                  rc, bytes);
               }
            }
         }
      }
      if (FD_ISSET(sfd, &rd_set))
      {  rc=ioctl(sfd, FIONREAD, &bytes);
         if (rc < 0)
         {  syslog(LOG_ERR, "func(ioctl): %m");
            break;
         }
         if (bytes==0)
         {  syslog(LOG_ERR, "server bytes = 0");
            break;
         }
         for (i=0; i<bytes/sizeof(buffer); i++)
         {  rc=recv(sfd, buffer, sizeof(buffer), 0);
            if (rc < sizeof(buffer))
            {  syslog(LOG_ERR, "func(recv): %s",
               rc < 0 ? strerror(errno): "(partial read)");
            }
            else
            {  write_log(2, buffer, sizeof(buffer));
               rc=send(cfd, buffer, sizeof(buffer), 0);
               if (rc < sizeof(buffer))
               {  syslog(LOG_ERR, "func(send): %s",
                  rc < 0 ? strerror(errno): "(partial write)");
               }
            }
         }
         bytes %= sizeof(buffer);
         if (bytes > 0)
         {  rc=recv(sfd, buffer, bytes, 0);
            if (rc < bytes)
            {  syslog(LOG_ERR, "func(recv): %s",
               rc < 0 ? strerror(errno): "(partial read)");
            }
            else
            {  write_log(2, buffer, bytes);
               rc=send(cfd, buffer, bytes, 0);
               if (rc < bytes)
               {  syslog(LOG_ERR, "func(send): %s",
                  rc < 0 ? strerror(errno): "(partial write)");
               }
            }
         }
      }
   }
}

void open_log()
{
   logfd=fopen(logname, "a");
}

void close_log()
{
   fclose(logfd);
}

void write_log(int peer, void * data, int size)
{
   if (logfd != NULL)
   {
      if (peer != lastpeer)
      {  fprintf(logfd, "\n<%s>: ", peer==1 ? "CLIENT":"SERVER");
         lastpeer=peer;
      }
      fwrite(data, sizeof(char), size, logfd);
      fflush(logfd);   
   }
}


int main(int argc, char ** argv)
{ 
   char            c;
   int             rc;

   openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);

/* TODO
 -p  1. Redefine own port
 -P  2. Redefine service port
 -A  3. Redefine service address
*/

#define OPTS "p:P:A:L"

   while ((c = getopt(argc, argv, OPTS)) != -1)
   {  switch (c)
      {
      case 'p':
         OwnService=strtol(optarg, NULL, 10);
         break;
      case 'P':
         Service=strtol(optarg, NULL, 10);
         break;
      case 'A':
         Address=optarg;
         break;
      case 'L':
         logname = optarg;
         break;
      default:
	 usage(-1);
      }
   }

   rc=daemon(0,0);
   if (rc != SUCCESS)
   {  syslog(LOG_ERR, "Can't daemonize, closing");
      exit(-1);
   }
   rc=link_wait(&client, OwnService);
   if (rc == -1)
   {  syslog(LOG_ERR, "link_wait(): %m");
      exit(-1);
   }
   rc=link_request(&server, Address, Service);
   if (rc == -1)
   {  syslog(LOG_ERR, "link_request(): %m");
      link_close(&client);
      exit(-1);
   }
   open_log();
   proxy(client.fd, server.fd);
   close_log();
   link_close(&client);
   link_close(&server);
   return 0;
}

void usage(int code)
{  syslog(LOG_ERR, "usage: %s %s", __progname, OPTS);
   closelog();
   exit(code);
}

