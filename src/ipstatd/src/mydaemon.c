/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

int
mydaemon(void)
{
	if (getppid() != 1) {
		signal(SIGTTOU, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);

		switch (fork()) {
		case -1:
		    return (-1);
		case 0:
		    break;
		default:
		    exit(0);
		}
		if (setsid() == -1)
			return (-1);
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	chdir("/");

	return (0);
}
