/*	$Id$	*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "ipstat.h"

int             sock_fd;

void
usage(char *progname)
{
	printf("Usage:\n\t%s [ -h host -p port ]\n", progname);
}

void
timeout()
{
	fprintf(stderr, "Connection timeout\n");
	exit(1);
}

int
do_auth()
{
	char            buf[100], *digest;
	char           *p, *d;
	MD5_CTX         ctx;
	int             n;

	if ((n = read(sock_fd, buf, sizeof(buf) - 1)) == -1) {
		perror("read");
		exit(1);
	}
	buf[n] = '\0';
#if DEBUG
	fprintf(stderr, "challenge: %s\n", buf);
#endif
	for (p = buf; isblank(*p); p++)
		continue;

	if (strncasecmp("CHAL", p, 4))
		return (-1);
	p += 4;
	while (isblank(*p))
		p++;
	d = p;
	while (isascii(*p) && !isspace(*p) && *p != '\0')
		p++;
	*p = '\0';

#if DEBUG
	fprintf(stderr, "challenge: %s, len: %d\n", d, p - d);
#endif

	MD5Init(&ctx);
	MD5Update(&ctx, (u_char *) d, p - d);
	MD5Update(&ctx, password, strlen(password));
	digest = MD5End(&ctx, NULL);
	snprintf(buf, sizeof(buf), "AUTH %s\n", digest);
#if DEBUG
	fprintf(stderr, "digest: %s, len: %d\n", buf, strlen(buf));
#endif
	if ((n = write(sock_fd, buf, strlen(buf))) == -1) {
		perror("write");
		exit(1);
	}
	if ((n = read(sock_fd, buf, sizeof(buf))) == -1) {
		perror("read");
		exit(1);
	}
	if (!strncmp(buf, "200", 3))
		return (0);
	return (-1);
}

void
sighndl(int sig)
{
	switch (sig) {
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
/*	    timeout();	*/
		break;
	    default:
		break;
	}
#ifdef  DEBUG
	if (sig != SIGALRM)
		syslog(LOG_DEBUG, "%d recived", sig);
#endif
}

int
sendcmd(char **argv)
{
	char            cmdbuf[MAXCMDLEN];
	char		*p;
	int             len;

	p = cmdbuf;
	for (p = cmdbuf; *argv; argv++) {
		len = snprintf(p, sizeof(cmdbuf), "%s ", *argv);
		if (len >= sizeof(cmdbuf)) {
			fprintf(stderr, "Command too long: %s\n", cmdbuf);
			exit(1);
		}
		p += len;
	}
	len = p - cmdbuf;
	cmdbuf[len - 1] = '\n';
	if (write(sock_fd, cmdbuf, len) == -1) {
		perror("write");
		exit(1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	extern int      optind;
	extern char    *optarg;
	struct sigaction sigact;
	int             c, n;
	int             sport = SERVER_PORT;
	char           *sname = "localhost";
	char            buf[4096];
	struct sockaddr_in sock_server;


	sigact.sa_handler = &sighndl;
	sigfillset(&sigact.sa_mask);
	sigact.sa_flags = SA_RESTART;
	sigaction(SIGALRM, &sigact, NULL);

	fclose(stdin);
	while ((c = getopt(argc, argv, "?h:p:sbl")) != -1)
		switch (c) {
		    case 's':
			break;
		    case 'b':
			break;
		    case 'l':
			break;
		    case 'h':
			sname = optarg;
			break;
		    case 'p':
			sport = atoi(optarg);
			break;
		    case '?':
			usage(argv[0]);
			exit(0);
		    default:
			usage(argv[0]);
			exit(1);
		}
	argc -= optind;
	argv += optind;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket");
		exit(1);
	}
	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(sport);
	sock_server.sin_addr.s_addr = getaddr(sname);

#if DEBUG
	fprintf(stderr, "host: %s, port: %hu\n",
		inet_ntoa(sock_server.sin_addr), sport);
#endif
	if (connect(sock_fd, (struct sockaddr *)&sock_server,
	    (socklen_t)sizeof(sock_server)) == -1) {
		perror("connect");
		exit(1);
	}
	signal(SIGALRM, (void *) &timeout);
	alarm(60);

	if (do_auth() == -1) {
		fprintf(stderr, "Can't login to statistic server.\n");
		exit(1);
	}
	sendcmd(argv);

	while ((n = read(sock_fd, buf, sizeof(buf))) > 0) {
		fwrite(buf, 1, n, stdout);
	}
	printf("\n");

	close(sock_fd);
	return (0);
}
