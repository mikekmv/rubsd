/*
 *		$Id$
 */

#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <md5.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "ipstat.h"

int	sock_fd;

int usage()
{
}

int timeout()
{
	fprintf(stderr,"Connection timeout \n");
	exit(1);
}

int do_auth()
{
	char	buf[100],*digest;
	char	*p,*d;
	MD5_CTX 		ctx;
	int	n;

	if ( (n = read(sock_fd,buf,sizeof(buf)-1)) == -1 ) {
		perror("read");
		exit(1);
	}
	*(buf + n) = '\0';
#ifdef DEBUG
	printf("challenge: %s\n",buf);
#endif
	for (p = buf; isblank(*p); p++ )
		continue;

	if (strncasecmp("CHAL",p,4))
		return(-1);
	p += 4;
	while ( isblank(*p) )
		p++;
	d = p; 
	while ( isascii(*p) && !isspace(*p) && (*p) != '\0' )
		p++;
	*p = '\0';

#ifdef DEBUG
	printf("chall: %s, len: %d\n",d,p-d);
#endif

	MD5Init(&ctx);
	MD5Update(&ctx, d,p-d);
	MD5Update(&ctx, password,strlen(password));
	digest = MD5End(&ctx,NULL);
	snprintf(buf,sizeof(buf),"AUTH %s\n",digest);
#ifdef DEBUG
	printf("digest: %s, len: %d\n",buf,strlen(buf));
#endif
	if ((n = write(sock_fd,buf,strlen(buf))) == -1 ) {
		perror("write");
		exit(1);
	}
	if ( (n = read(sock_fd,buf,sizeof(buf))) == -1 ) {
		perror("read");
		exit(1);
	} 
	if(!strncmp(buf,"200",3))
		return(0);
	return(-1);	
}

void sighndl(sig)
int     sig;
{
        switch(sig) {
            case SIGPIPE:
                break;
            case SIGTERM:
                break;
            case SIGHUP:
                break;
            case SIGINT:
                break;
            case SIGUSR1:
                break;
            case SIGUSR2:
                break;
            case SIGALRM:
/*		timeout();	*/
                break;
            default:
                break;
        }
#ifdef  DEBUG
        if (sig != SIGALRM )
                syslog(LOG_DEBUG,"%d recived",sig);
#endif
}

int main(argc, argv)
int argc;
char *argv[];
{
        extern  int     optind;
        extern  char    *optarg;
	struct  sigaction       sigact;
	int	c,n;
	char	command[16],buf[4096];
	char	r='\n';
	struct sockaddr_in     	sock_server;
        struct sockaddr_in      sock_client;


        sigact.sa_handler = &sighndl;
        sigfillset(&sigact.sa_mask);
        sigact.sa_flags = SA_RESTART;
        sigaction(SIGALRM,&sigact,NULL);

	fclose(stdin);
	while ((c = getopt(argc, argv, "?hpbl")) != -1)
		switch (c) {
	                case 'p' :
				strncpy(command,"stat",sizeof(command));
				strncat(command,&r,sizeof(command));
       		                break;
                	case 'b' :
                        	break;
                	case 'l' :
                        	break;
                	case 'h' :
                	case '?' :
                        	usage(argv[0]);
				exit(0);
                	default :
                        	usage(argv[0]);
				exit(1);
                }

        if( (sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1 ) {
                perror("socket");
                exit(1);
        }

        sock_server.sin_family = AF_INET;
        sock_server.sin_port = htons(SERVER_PORT);
        sock_server.sin_addr.s_addr = inet_addr("194.85.82.144");

	if ( connect(sock_fd, (struct sockaddr *)&sock_server,
				 (socklen_t)sizeof(sock_server)) == -1 ) {
		perror("connect");
		exit(1);
	}

	signal(SIGALRM,(void *)&timeout);
	alarm(60);

	if (do_auth() == -1) {
		fprintf(stderr,"Can't login to stat server.\n");
		exit(1);
	}
	if (write(sock_fd,command,strlen(command)) == -1 ) {
		perror("write");
		exit(1);
	}
	while ( (n = read(sock_fd,buf,sizeof(buf))) > 0 ) {
		fwrite(buf,1,n,stdout);
	}
	printf("\n");
	close(sock_fd);
}

