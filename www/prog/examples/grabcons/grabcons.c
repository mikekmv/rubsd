/*
 * ������ ������ ������������� ���������� ��������� � �������� �� �� stdout.
 *
 * $RuOBSD$
 */

#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <util.h>
#include <unistd.h>


#define CONS_BUFSIZE		2048	/* ������ I/O ������ */


int main(void);


int
main(void)
{
	char buf[CONS_BUFSIZE];
	ssize_t nbytes;
	int tty_fd, pty_fd, on = 1;

	/* ��������� ���� tty-pty */
	if (openpty(&pty_fd, &tty_fd, NULL, NULL, NULL) < 0)
		err(EX_UNAVAILABLE, "openpty");

	/* ������������ ���������� ��������� � tty */
	if (ioctl(tty_fd, TIOCCONS, &on) < 0)
		err(EX_UNAVAILABLE, "TIOCCONS");

	/* ������ ���������� ��������� �� pty � ����� � stdout */
	while ((nbytes = read(pty_fd, buf, sizeof(buf))) >= 0)
		if (write(STDOUT_FILENO, buf, nbytes) < 0)
			err(EX_IOERR, "write");

	err(EX_IOERR, "read");
	/* NOTREACHED */

	return (EX_OK);
}
