/*
 *		$Id$
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>


void mydaemon(void)
{
	int fd;
	struct rlimit filelimit;

	if (getppid() != 1){
	  signal(SIGTTOU, SIG_IGN);
	  signal(SIGTTIN, SIG_IGN);
	  signal(SIGTSTP, SIG_IGN);
	  
	  if( fork() != 0)
	    exit(0);
	  setsid();
	}

	getrlimit(RLIMIT_NOFILE,&filelimit);
	for( fd=0 ; fd<filelimit.rlim_max ; fd++ )
	  close(fd);

	chdir("/");
}
