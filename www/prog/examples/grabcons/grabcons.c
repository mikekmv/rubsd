/*
 * Данный пример перехватывает консольные сообщения и печатает их на stdout.
 *
 * $RuOBSD$
 */

#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <util.h>
#include <unistd.h>


#define CONS_BUFSIZE		2048	/* размер I/O буфера */


int main(void);


int
main(void)
{
	char buf[CONS_BUFSIZE];
	ssize_t nbytes;
	int tty_fd, pty_fd, on = 1;

	/* открываем пару tty-pty */
	if (openpty(&pty_fd, &tty_fd, NULL, NULL, NULL) < 0)
		err(EX_UNAVAILABLE, "openpty");

	/* переадресуем консольные сообщения в tty */
	if (ioctl(tty_fd, TIOCCONS, &on) < 0)
		err(EX_UNAVAILABLE, "TIOCCONS");

	/* читаем консольные сообщения из pty и пишем в stdout */
	while ((nbytes = read(pty_fd, buf, sizeof(buf))) >= 0)
		if (write(STDOUT_FILENO, buf, nbytes) < 0)
			err(EX_IOERR, "write");

	err(EX_IOERR, "read");
	/* NOTREACHED */

	return (EX_OK);
}
