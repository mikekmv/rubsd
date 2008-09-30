/*
 * ������ ������ ������������� ������� ������ ������������ UDP
 * ������� � ����������� ���������. �������� ������������ ��������
 * ����, ��� ����� ������ ������, ������ ������ ��� � �� ��� ������� ��
 * ������.
 *
 * ��������� �������� � ���� �������:
 * - ����� ������� (��������� ���� ������ � �������� ��� �� ������
 *   ���� ����� ��������� ��������)
 * - ����� ������� (��������� ���������� ����������� ����� �������)
 *
 * ������:
 *	1. ������� ���� ��� ��������:
 *
 *		dd if=/dev/arandom of=sign.key bs=2048 count=1
 *
 *	2. ��������� ������:
 *
 *		signpkt -k sign.key -s
 *
 *	3. ��������� ������ �������:
 *
 *		signpkt -k sign.key ������ [...]
 *
 * � ������ ���������� ������ �� ������� � �������, ������ ����� ��������
 * ���������� ������ ������. � ������ ������������ �����, ������ �����
 * �������� ��������� � ���������� �������.
 *
 * ��� �������� � ��������� ������������ localhost � ���� 32768. ��������
 * ����� ���� �������� ����������.
 *
 * $RuOBSD$
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/hmac.h>
#include <err.h>
#include <sha1.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>


#define UDP_PORT	32768		/* UDP ���� */
#define MAX_PKTSIZE	8192		/* ������������ ������ ������ */


struct pkthdr {
	size_t		ph_len;
	u_int8_t	ph_hmac[SHA1_DIGEST_LENGTH];
};

struct packet {
	struct pkthdr	p_hdr;
#define p_len		p_hdr.ph_len
#define p_hmac		p_hdr.ph_hmac
	char		p_data[MAX_PKTSIZE];
};


static char *keyfile;
static int server;


static __dead void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr,
	    "usage: %s [-k keyfile] [-s]\n"
	    "       %s [-k keyfile] string [...]\n",
	    __progname, __progname);
	exit(EX_USAGE);
}

int
main(int argc, char *const argv[])
{
	struct packet pkt;
	struct sockaddr_in sin;
	char *key = "";
	HMAC_CTX ctx;
	ssize_t size;
	int ch, s;

	while ((ch = getopt(argc, argv, "k:s")) != -1)
		switch (ch) {
		case 'k':
			keyfile = optarg;
			break;
		case 's':
			server++;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	/* ��������� SHA1 ����� ����� */
	if (keyfile != NULL && (key = SHA1File(keyfile, NULL)) == NULL)
		err(EX_UNAVAILABLE, "SHA1File: %s", keyfile);

	/* ��������� UDP socket */
	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		err(EX_UNAVAILABLE, "socket");

	/* �������������� sockaddr ��������� */
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_port = htons(UDP_PORT);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (server) {
		/* ������ */

		/* ��������� �������� � ������/����� */
		if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
			err(EX_UNAVAILABLE, "bind");

		for (;;) {
			u_int8_t hmac[SHA1_DIGEST_LENGTH];

			/* ����� ������ */
			if ((size = recv(s, &pkt, sizeof(pkt), 0)) < 0)
				err(EX_IOERR, "recv");

			/* �������� �� ����������� */
			if ((size_t)size < sizeof(struct pkthdr)) {
				warnx("recv: short read");
				continue;
			}
			if (pkt.p_len != (size_t)size) {
				warnx("recv: bad packet header");
				continue;
			}

			/* ��������� ������� ������ */
			bcopy(pkt.p_hmac, hmac, sizeof(hmac));

			/* ��������� ��������� ������� ������ � ����� ������ */
			bzero(pkt.p_hmac, sizeof(pkt.p_hmac));
			HMAC_CTX_init(&ctx);
			HMAC_Init(&ctx, key, (int)strlen(key), EVP_sha1());
			HMAC_Update(&ctx, (const u_char *)&pkt, pkt.p_len);
			HMAC_Final(&ctx, pkt.p_hmac, (u_int *)&ch);

			/* ���������� ������� */
			if (bcmp(hmac, pkt.p_hmac, sizeof(hmac)) != 0)
				warnx("recv: invalid packet signature");
			else
				(void)write(STDOUT_FILENO, pkt.p_data,
				    pkt.p_len - sizeof(struct pkthdr));
		}
	} else {
		/* ������ */

		/* ��������� ��������� ��������� ������ � ����� */
		pkt.p_data[0] = '\0';
		for (ch = 0; ch < argc; ch++) {
			if (ch != 0)
				(void)strlcat(pkt.p_data, " ",
				    sizeof(pkt.p_data));
			(void)strlcat(pkt.p_data, argv[ch],
			    sizeof(pkt.p_data));
		}
		(void)strlcat(pkt.p_data, "\n", sizeof(pkt.p_data));

		/* ������������� ����� ������ */
		pkt.p_len = strlen(pkt.p_data) + sizeof(struct pkthdr);

		/* ��������� ������� � ����� ������ */
		bzero(pkt.p_hmac, sizeof(pkt.p_hmac));
		HMAC_CTX_init(&ctx);
		HMAC_Init(&ctx, key, (int)strlen(key), EVP_sha1());
		HMAC_Update(&ctx, (const u_char *)&pkt, pkt.p_len);
		HMAC_Final(&ctx, pkt.p_hmac, (u_int *)&ch);

		/* ���������� ����� */
		if (sendto(s, &pkt, pkt.p_len, 0, (struct sockaddr *)&sin,
		    sizeof(sin)) < 0)
			err(EX_IOERR, "sendto");
	}

	return (EX_OK);
}
