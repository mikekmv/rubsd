/*
 * Данный пример демонстрирует использование libevent и getaddrinfo для
 * использования в  серверных  программах. Программа обслуживает telnet
 * соединения на указанных портах/адресах и передает весь ввод сессии в
 * другие сессии. Если сессия неактивна в  течение  60  секунд,  сессия
 * завершается по таймауту.
 *
 * Запуск:
 *	eventsrv [-46d] [[адрес][:порт] ...]
 *
 *	адрес	- адрес, к которому выполнять привязку (если не указан
 *		  выполняется привязка ко всем адресам)
 *	порт	- порт, к которому выполнять привязку (если не указан
 *		  выполняется привязка к порту 32768)
 *
 *	-4	- использовать только IPv4 адреса
 *	-6	- использовать только IPv6 адреса
 *	-d	- не отцепляться от терминала, выводить лог на stderr
 *
 * $RuOBSD$
 */
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <syslog.h>
#include <unistd.h>


#define TIMEOUT			60		/* таймаут сессии в секундах */
#define BUFSIZE			16384		/* размер буфера I/O */
#define TCP_BACKLOG		5		/* размер очереди listen */
#define TCP_PORT		"32768"		/* порт по умолчанию */


struct session {
	struct bufferevent	*s_bev;
	u_int32_t		s_id;
	LIST_ENTRY(session)	s_entry;
};


int main(int, char **);
__dead static void usage(void);
static void srv_conn(int, short, void *);
static void srv_read(struct bufferevent *, void *);
static void srv_write(struct bufferevent *, void *);
static void srv_error(struct bufferevent *, short, void *);


extern char *__progname;

static LIST_HEAD(, session) sessions = LIST_HEAD_INITIALIZER(&sessions);
static u_int32_t sid;

static char timeout[] = "\r\n*** connection timed out ***\r\n";

static int inet4;
static int inet6;
static int debug;


int
main(int argc, char *argv[])
{
	const char *cause = NULL;
	int ch, nsocks = 0, save_errno = 0;

	while ((ch = getopt(argc, argv, "46d")) != -1)
		switch (ch) {
		case '4':
			inet4++;
			break;
		case '6':
			inet6++;
			break;
		case 'd':
			debug++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		argc++;

	if (inet4 && inet6)
		errx(EX_CONFIG, "Can't specify both -4 and -6");

	/* инициализируем подсистему событий */
	(void)event_init();

	/* выполняем привязку к указанным адресам/портам */
	for (ch = 0; ch < argc; ch++) {
		struct addrinfo hints, *res, *res0;
		struct event *ev;
		char *addr, *port;
		int error, s, on = 1;

		addr = NULL;
		port = TCP_PORT;
		if (argv[ch] != NULL) {
			if (argv[ch][0] != ':')
				addr = argv[ch];
			if ((port = strchr(argv[ch], ':')) != NULL)
				*port++ = '\0';
			else
				port = TCP_PORT;
		}

		bzero(&hints, sizeof(hints));
		if (inet4 || inet6)
			hints.ai_family = inet4 ? PF_INET : PF_INET6;
		else
			hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		if ((error = getaddrinfo(addr, port, &hints, &res0)) != 0)
			err(EX_UNAVAILABLE, "%s", gai_strerror(error));

		for (res = res0; res != NULL; res = res->ai_next) {
			if ((s = socket(res->ai_family, res->ai_socktype,
			    res->ai_protocol)) < 0) {
				cause = "socket";
				save_errno = errno;
				continue;
			}

			if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on,
			    sizeof(on)) < 0) {
				cause = "SO_REUSEADDR";
				save_errno = errno;
				(void)close(s);
				continue;
			}

			if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
				cause = "bind";
				save_errno = errno;
				(void)close(s);
				continue;
			}

			(void)listen(s, TCP_BACKLOG);

			if ((ev = malloc(sizeof(*ev))) == NULL)
				err(EX_UNAVAILABLE, NULL);

			/* добавляем сервер к подистеме событий */
			event_set(ev, s, EV_READ | EV_PERSIST, srv_conn, ev);
			if (event_add(ev, NULL) < 0)
				err(EX_UNAVAILABLE, "event_add");

			nsocks++;
		}

		freeaddrinfo(res0);
	}

	if (nsocks == 0) {
		errno = save_errno;
		err(EX_UNAVAILABLE, "%s", cause);
		/* NOTREACHED */
	}

	/* устанавливаем параметры syslog */
	if (debug)
		openlog(__progname, LOG_PID | LOG_PERROR, LOG_DAEMON);
	else
		openlog(__progname, LOG_PID, LOG_DAEMON);

	/* отключаемся от терминала если не был указан -d */
	if (!debug && daemon(0, 0) < 0)
		err(EX_OSERR, NULL);

	/* передаем управление диспетчеру подсистемы событий */
	(void)event_dispatch();

	return (EX_OK);
}

__dead static void
usage(void)
{
	(void)fprintf(stderr, "usage: %s [-46d] [[addr][:port] ...]\n",
	    __progname);
	exit(EX_USAGE);
}

/*
 * Данная подпрограмма вызывается в момент, когда клиент подключается к
 * серверу.
 */
static void
srv_conn(int fd, short event __attribute__((__unused__)),
    void *arg __attribute__((__unused__)))
{
	char addr[INET6_ADDRSTRLEN];
	struct sockaddr_storage ss;
	struct bufferevent *bev;
	struct session *sess;
	socklen_t ss_len = sizeof(ss);
	int s;

	ss.ss_len = ss_len;
	if ((s = accept(fd, (struct sockaddr *)&ss, &ss_len)) < 0) {
		syslog(LOG_ERR, "accept: %m");
		return;
	}

	switch (ss.ss_family) {
	case AF_INET:
		(void)inet_ntop(AF_INET,
		    &((struct sockaddr_in *)&ss)->sin_addr, addr,
		    sizeof(addr));
		syslog(LOG_NOTICE, "(%u) connection from %s:%u", sid, addr,
		    ntohs(((struct sockaddr_in *)&ss)->sin_port));
		break;
	case AF_INET6:
		(void)inet_ntop(AF_INET6,
		    &((struct sockaddr_in6 *)&ss)->sin6_addr, addr,
		    sizeof(addr));
		syslog(LOG_NOTICE, "(%u) connection from %s:%u", sid, addr,
		    ntohs(((struct sockaddr_in6 *)&ss)->sin6_port));
		break;
	}

	if ((sess = malloc(sizeof(*sess))) == NULL) {
		syslog(LOG_ERR, "malloc: %m");
		(void)close(s);
		return;
	}

	if ((bev = bufferevent_new(s, srv_read, srv_write,
	    srv_error, sess)) == NULL) {
		syslog(LOG_ERR, "bufferevent_new: %m");
		(void)close(s);
		free(sess);
		return;
	}

	sess->s_bev = bev;
	sess->s_id = sid++;
	LIST_INSERT_HEAD(&sessions, sess, s_entry);

	/* регистрируем сессию в подсистеме событий */
	bufferevent_settimeout(bev, TIMEOUT, 0);
	(void)bufferevent_enable(bev, EV_READ | EV_TIMEOUT);
}

/*
 * Данная функция вызывается когда есть данные от клиента.
 */
static void
srv_read(struct bufferevent *bev, void *arg)
{
	char buf[BUFSIZE];
	struct session *sess;
	size_t len;

	/* читаем данные от клиента и передаем остальным клиентам */
	while ((len = bufferevent_read(bev, buf, sizeof(buf))) != 0)
		LIST_FOREACH(sess, &sessions, s_entry)
			if (sess != arg)
				(void)bufferevent_write(sess->s_bev, buf, len);
}

/*
 * Дананя функция вызывается при выводе данных клиенту.
 */
static void
srv_write(struct bufferevent *bev __attribute__((__unused__)),
    void *arg __attribute__((__unused__)))
{
	/* NOTHING */
}

/*
 * Данная функция вызывается в случае возникновения ошибки,
 * закрытия сессии клиентом или по достижении таймаута сессии.
 */
static void
srv_error(struct bufferevent *bev, short what, void *arg)
{
	struct session *sess = arg;

	if (what & EVBUFFER_EOF)
		syslog(LOG_NOTICE, "(%d) connection closed", sess->s_id);
	else if (what == (EVBUFFER_ERROR | EVBUFFER_READ))
		syslog(LOG_NOTICE, "(%d) connection reset", sess->s_id);
	else if (what & EVBUFFER_TIMEOUT) {
		if (fcntl(bev->ev_write.ev_fd, F_SETFL, O_NONBLOCK) == 0)
			(void)write(bev->ev_write.ev_fd, timeout,
			    sizeof(timeout) - 1);
		syslog(LOG_NOTICE, "(%d) connection timed out", sess->s_id);
	} else if (what & EVBUFFER_WRITE)
		syslog(LOG_ERR, "(%d) connection write error", sess->s_id);
	else
		syslog(LOG_ERR, "(%d) unknown connection error", sess->s_id);

	LIST_REMOVE(sess, s_entry);
	(void)close(sess->s_bev->ev_write.ev_fd);
	free(sess);
}
