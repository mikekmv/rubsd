/*
 * ������ ��� ��������� �������� � sendmail � �������������� Milter API.
 * ��  ��  ���������  �������  ��������  � �������� ������ ����������� �
 * �������� �������.
 *
 * ������ �������: milter socket
 * ��� socket ����� ���� ������ � ���� "inet:port[@address]" (TCP) ���
 * "local:path" (UNIX).
 *
 * $RuOBSD$
 */
#include <sys/types.h>
#include <libmilter/mfapi.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>


int main(int, char * const *);
__dead static void usage(void);

static sfsistat mlfi_connect(SMFICTX *, char *, _SOCK_ADDR *);
static sfsistat mlfi_helo(SMFICTX *, char *);
static sfsistat mlfi_envfrom(SMFICTX *, char **);
static sfsistat mlfi_envrcpt(SMFICTX *, char **);
static sfsistat mlfi_header(SMFICTX *, char *, char *);
static sfsistat mlfi_eoh(SMFICTX *);
static sfsistat mlfi_body(SMFICTX *, unsigned char *, size_t);
static sfsistat mlfi_eom(SMFICTX *);
static sfsistat mlfi_abort(SMFICTX *);
static sfsistat mlfi_close(SMFICTX *);


struct smfiDesc smfilter = {
	"milter",			/* �������� */

	SMFI_VERSION,			/* ������ API */
	0,				/* ����� */

	/*
	 * ����� ��������� ��������:
	 *
	 *	SMFIF_ADDHDRS	- ������ ����� ��������� ���� ���������
	 *	SMFIF_CHGHDRS	- ������ ����� ������ �/��� ������� ����
	 *			  ���������
	 *	SMFIF_CHGBODY	- ������ ����� �������� ���� ���������
	 *	SMFIF_ADDRCPT	- ������ ����� ��������� �����������
	 *	SMFIF_DELRCPT	- ������ ����� ������� �����������
	 *	SMFIF_QUARANTINE- ������ ����� ��������� ��������� ����������
	 */

	mlfi_connect,			/* ������ ����������� */
	mlfi_helo,			/* ������ HELO */
	mlfi_envfrom,			/* ������ ����������� */
	mlfi_envrcpt,			/* ������ ���������� */
	mlfi_header,			/* ������ ��������� */
	mlfi_eoh,			/* ������ ����� ��������� */
	mlfi_body,			/* ������ ���� ��������� */
	mlfi_eom,			/* ������ ����� ��������� */

	mlfi_abort,			/* ������� ���������� ��������� */
	mlfi_close			/* ������� ���������� ��������� */
};


int
main(int argc, char * const *argv)
{
	if (argc != 2)
		usage();

	/* ������� socket ������� */
	(void)smfi_setconn(argv[1]);

	/* ���������������� ������ */
	if (smfi_register(smfilter) == MI_FAILURE)
		errx(EX_UNAVAILABLE, "smfi_register failed");

	return (smfi_main());
}

__dead static void
usage(void)
{
	extern char *__progname;

	(void)fprintf(stderr, "usage: %s sock\n", __progname);
	exit(EX_USAGE);
}

/*
 * ���� ���� �������� �������, ��������� ��� ������������� ������
 * ������� �������. ����� �� ���������� �������� ������ ����� �������.
 * ����� ��������� ���������� �������� ���:
 *	http://www.milter.org/milter_api/api.html
 *
 * ������� Milter API, ��������� ��� �������������:
 *
 * ������� ������� � ������:
 *
 * char *smfi_getsymval(SMFICTX *ctx, char *symname)
 *	- ���������� �������� ������� conf_MILTER_MACROS_��� (NULL ���� ��
 *	  ���������)
 *		symname	- ��� �������
 * void *smfi_getpriv(SMFICTX *ctx)
 *	- ���������� ���������� ������ ������� (��. smfi_setpriv)
 *
 * int smfi_setpriv(SMFICTX *ctx, void *data)
 *	- ������������� ���������� ������ ������� (���������� MI_SUCCESS
 *	  ��� MI_FAILURE)
 *		data	- ��������� �� ���������� ������
 *
 * int smfi_setreply(SMFICTX *ctx, char *rcode, char *xcode, char *message)
 *	- ������������� ��� SMTP ������ ��� �������� (������ 4xx ��� 5xx,
 *	  ���������� MI_SUCCESS ��� MI_FAILURE)
 *		rcode	- RFC 821/2821 ���
 *		xcode	- 1893/2034 ���������� ��� (����� ���� NULL)
 *		message	- ��������� �� ������
 *
 * int smfi_setmreply(SMFICTX *ctx, char *rcode, char *xcode, ...)
 *	- ������������� ��� ������������� SMTP ������ ��� ��������
 *	  (������ 4xx ��� 5xx, ���������� MI_SUCCESS ��� MI_FAILURE)
 *		rcode	- RFC 821/2821 ���
 *		xcode	- 1893/2034 ���������� ��� (����� ���� NULL)
 *		...	- ������ ��������� (�� 32)
 *
 * ������� ����������� ��������� (���������� MI_SUCCESS ��� MI_FAILURE):
 *
 * int smfi_addheader(SMFICTX *ctx, char *headerf, char *headerv)
 *	- ��������� ���� ��������� (���������� ������ �� mlfi_eom,
 *	  ��������� ���� SMFIF_ADDHDRS)
 *		headerf	- ��� ���� ���������
 *		headerv - �������� ���� ���������
 *
 * int smfi_chgheader(SMFICTX *ctx, char *headerf, mi_int32 hdridx,
 *   char *headerv)
 *	- �������� ���� ��������� (���������� ������ �� mlfi_eom,
 *	  ��������� ���� SMFIF_CHGHDRS)
 *		headerf	- ��� ���� ���������
 *		hdridx	- ����� ��������� ���� (������� � 1); ����
 *			  ����� ������ ����� ��������� - �����������
 *			  ����� ����
 *		headerv - �������� ���� ��������� (NULL - ������� ����)
 *
 * int smfi_insheader(SMFICTX *ctx, mi_int32 hdridx, char *headerf,
 *   char *headerv)
 *	- ��������� ���� ��������� (���������� ������ �� mlfi_eom,
 *	  ��������� ���� SMFIF_ADDHDRS)
 *		hdridx	- ���������� ����� � ������ (0 - � ����� ������)
 *		headerf	- ��� ���� ���������
 *		headerv - �������� ���� ���������
 *
 * int smfi_addrcpt(SMFICTX *ctx, char *rcpt)
 *	- ��������� ���������� (���������� ������ �� mlfi_eom,
 *	  ��������� ���� SMFIF_ADDRCPT)
 *		rcpt	- ����������
 *
 * int smfi_delrcpt(SMFICTX *ctx, char *rcpt)
 *	- ������� ���������� (���������� ������ �� mlfi_eom,
 *	  ��������� ���� SMFIF_DELRCPT)
 *		rcpt	- ����������
 *
 * int smfi_replacebody(SMFICTX *ctx, unsigned char *bodyp, int bodylen)
 *	- �������� ���� ������ (���������� ������ �� mlfi_eom, ���������
 *	  ���� SMFIF_CHGBODY)
 *		bodyp	- ������� ��������� ���� ���������
 *		bodylen	- �����
 *
 * ������� ����� � sendmail (���������� MI_SUCCESS ��� MI_FAILURE):
 *
 * int smfi_progress(SMFICTX *ctx)
 *	- �������� sendmail'� ��� ��������� ��������� ��� ���� � ����������
 *	  ������� ����-���� (���������� ������ �� mlfi_eom)
 *
 * int smfi_quarantine(SMFICTX *ctx, char *reason)
 *	- �������� ��������� �� �������� � ��������� ������� (����������
 *	  ������ �� mlfi_eom, ��������� ���� SMFIF_QUARANTINE)
 *		reason	- ������� ���������� �� ��������
 */

/*
 * ������ �����������. ���������� � ������ ����������� �������.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *	hostname	- ��� ����� ������� ��� IP ����� � ���� [xx.yy.zz.tt]
 *	hostaddr	- ��������� sockaddr ���������� �� getpeername ���
 *			  NULL ���� ����������� ������������ � stdin
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ����������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 */
static sfsistat
mlfi_connect(SMFICTX *ctx, char *hostname, _SOCK_ADDR *hostaddr)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ HELO. ���������� ����� ������ �������� HELO/EHLO.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *	helohost	- �������� ������� HELO ��� EHLO
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ����������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 */
static sfsistat
mlfi_helo(SMFICTX *ctx, char *helohost)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ �����������. ���������� ����� ������ ������� MAIL FROM.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *	argv		- ������ ���������� �������, ����������� NULL
 *			  (argv[0] �������� ����� �����������)
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ����������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 */
static sfsistat
mlfi_envfrom(SMFICTX *ctx, char **argv)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ ����������. ���������� ����� ������ ������� RCPT TO.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *	argv		- ������ ���������� �������, ����������� NULL
 *			  (argv[0] �������� ����� ����������)
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� �������� ����������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������ ��� �������� ����������
 *	SMFIS_DISCARD	- ������������ ��������� ��� ���������� ��������,
 *			  �� ��������� ���� ������
 */
static sfsistat
mlfi_envrcpt(SMFICTX *ctx, char **argv)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ ���������. ���������� ����� ������ ������� RCPT TO.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *	headerf		- �������� ���� ���������
 *	headerv		- �������� ���� ��������� (��� CR/LF � �����)
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ���������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 *	SMFIS_DISCARD	- ������������ ��������� ��� ���������� ��������,
 *			  �� ��������� ���� ������
 */
static sfsistat
mlfi_header(SMFICTX *ctx, char *headerf, char *headerv)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ ����� ���������. ���������� ����� ��������� ���������.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ���������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 *	SMFIS_DISCARD	- ������������ ��������� ��� ���������� ��������,
 *			  �� ��������� ���� ������
 */
static sfsistat
mlfi_eoh(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ ���� ���������. ���������� ���� ��� ��������� ��� ����� mkfi_eoh
 * � ����� mlfi_eom.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *	bodyp		- ��������� �� ������� ���� ����
 *	len		- ����� �������� �����
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ���������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 *	SMFIS_DISCARD	- ������������ ��������� ��� ���������� ��������,
 *			  �� ��������� ���� ������
 */
static sfsistat
mlfi_body(SMFICTX *ctx, unsigned char *bodyp, size_t len)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������ ����� ���������. ���������� ����� ��������� ���� ���������.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ���������� ��������� ���������
 *	SMFIS_REJECT	- ���������� ���������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 *	SMFIS_DISCARD	- ������������ ��������� ��� ���������� ��������,
 *			  �� ��������� ���� ������
 */
static sfsistat
mlfi_eom(SMFICTX *ctx)
{
	smfi_addheader(ctx, "X-Shit-Happens", "Yes");

	return (SMFIS_CONTINUE);
}

/*
 * ������� ���������� ���������. ���������� ���� ��������� ���������
 * ���������� �� ���������� � �� ��� ��������� ��� SMFIS_REJECT,
 * SMFIS_ACCEPT ��� SMFIS_DISCARD.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ����������
 */
static sfsistat
mlfi_abort(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * ������� ���������� ���������. ���������� ���� ��������� ���������
 * ���������.
 *
 * ���������� ���������:
 *	ctx		- ���������, ������������ � ������� milter
 *
 * ���� ��������:
 *	SMFIS_CONTINUE	- ����������
 *	SMFIS_REJECT	- ���������� ���������
 *	SMFIS_ACCEPT	- ������� ��������� ��� ���������� ��������
 *	SMFIS_TEMPFAIL	- ������� ��������� ������
 *			  �� ��������� ���� ������
 */
static sfsistat
mlfi_close(SMFICTX *ctx)
{
	return (SMFIS_ACCEPT);
}
