/*
 * Данный пример демонстрирует использование kevent для отслеживания
 * событий, происходящих с заданными PID или файлами/каталогами.
 *
 * Запуск:
 *	kevent file ...
 *		- для отслеживания событий, происходящих с указанными
 *		  файлами/каталогами
 *	kevent -p pid ...
 *		- для отслеживания событий, происходящих с указанными
 *		  PID.
 * $RuOBSD$
 */

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>


static int watch_pids;


int main(int, char **);
__dead static void usage(void);


int
main(int argc, char **argv)
{
	struct kevent *kev;
	int i, n, ch, kq, nevents;

	while ((ch = getopt(argc, argv, "p")) != -1)
		switch (ch) {
		case 'p':
			watch_pids++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		usage();

	/* создаем очередь событий */
	if ((kq = kqueue()) < 0)
		err(EX_UNAVAILABLE, "kqueue");

	/* резервируем память для описателей событий */
	if ((kev = malloc(sizeof(struct kevent) * argc)) == NULL)
		err(EX_UNAVAILABLE, "malloc");

	for (i = nevents = 0; i < argc; i++) {
		const char *errstr;

		if (watch_pids) {
			ch = strtonum(argv[i], 1, 65535, &errstr);
			if (errstr != NULL) {
				warnx("%s: %s", argv[i], errstr);
				continue;
			}

			/* устанавливаем типы событий, за которыми наблюдаем */
			EV_SET(&kev[nevents], ch, EVFILT_PROC,
			    EV_ADD | EV_CLEAR, NOTE_EXIT | NOTE_FORK |
			    NOTE_EXEC | NOTE_TRACK, NULL, argv[i]);
		} else {
			if ((ch = open(argv[i], O_RDONLY)) < 0) {
				warn("open: %s", argv[i]);
				continue;
			}
			/* устанавливаем типы событий, за которыми наблюдаем */
			EV_SET(&kev[nevents], ch, EVFILT_VNODE,
			    EV_ADD | EV_CLEAR, NOTE_DELETE | NOTE_WRITE |
			    NOTE_EXTEND | NOTE_TRUNCATE | NOTE_ATTRIB |
			    NOTE_LINK | NOTE_RENAME | NOTE_REVOKE,
			    NULL, argv[i]);
		}
		nevents++;
	}

	ch = nevents;
	for (;;) {
		printf("Waiting for an event... ");
		fflush(stdout);
		/* ждем события */
		if ((n = kevent(kq, kev, ch, kev, nevents, NULL)) < 0)
			err(EX_UNAVAILABLE, "kevent");
		ch = 0;
		printf("\n");
		if (watch_pids) {
			for (i = 0; i < n; i++) {
				printf("\t%s:", (char *)kev[i].udata);
				if (kev[i].fflags & NOTE_EXIT)
					printf(" EXIT");
				if (kev[i].fflags & NOTE_FORK)
					printf(" FORK");
				if (kev[i].fflags & NOTE_CHILD)
					printf(" CHILD(%d)", kev[i].data);
				if (kev[i].fflags & NOTE_EXEC)
					printf(" EXEC");
				if (kev[i].fflags & NOTE_TRACKERR)
					printf(" TRACKERR");
			}
		} else {
			for (i = 0; i < n; i++) {
				printf("\t%s:", (char *)kev[i].udata);
				if (kev[i].fflags & NOTE_DELETE)
					printf(" DELETE");
				if (kev[i].fflags & NOTE_WRITE)
					printf(" WRITE");
				if (kev[i].fflags & NOTE_EXTEND)
					printf(" EXTEND");
				if (kev[i].fflags & NOTE_TRUNCATE)
					printf(" TRUNCATE");
				if (kev[i].fflags & NOTE_ATTRIB)
					printf(" ATTRIB");
				if (kev[i].fflags & NOTE_LINK)
					printf(" LINK");
				if (kev[i].fflags & NOTE_RENAME)
					printf(" RENAME");
				if (kev[i].fflags & NOTE_REVOKE)
					printf(" REVOKE");
			}
		}
		printf("\n");
		fflush(stdout);
	}

	return (EX_OK);
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s file [...]\n       %s -p pid [...]\n",
	    __progname, __progname);
	exit(EX_USAGE);
}
