/*	$RuOBSD: execwrap.c,v 1.1 2002/10/27 21:01:42 grange Exp $	*/

#include <sys/types.h>

#include <err.h>
#include <login_cap.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

__dead static void usage(void);

int main(int argc, char *argv[])
{
	struct passwd *pw;
	char *user = NULL;
	int noenv = 0, ch;

	while ((ch = getopt(argc, argv, "eu:")) != -1)
		switch (ch) {
		case 'e':
			noenv = 1;
			break;
		case 'u':
			user = optarg;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();
		/* NOTREACHED */

	if (user == NULL)
		usage();
		/* NOTREACHED */

	if ((pw = getpwnam(user)) == NULL)
		errx(1, "%s: No such user", user);

	if (!noenv)
		if (setenv("USER", pw->pw_name, 1) == -1 ||
		    setenv("LOGNAME", pw->pw_name, 1) == -1 ||
		    setenv("HOME", pw->pw_dir, 1) == -1 ||
		    setenv("SHELL", pw->pw_shell, 1) == -1)
			err(1, "setenv()");

	if (setusercontext(NULL, pw, pw->pw_uid, LOGIN_SETALL) == -1)
		err(1, "setusercontext()");

	(void)execvp(*argv, argv);
	err(1, "execvp: %s", *argv);

	return 0;
}

__dead static void
usage()
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-e] -u user command\n", __progname);
	exit(1);
}
