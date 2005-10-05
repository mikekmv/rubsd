/*
 * Шаблон для написания фильтров к sendmail с использованием Milter API.
 * Он  не  выполняет  никаких  действий  и содержит только определения и
 * описания функций.
 *
 * Запуск фильтра: milter socket
 * где socket может быть указан в виде "inet:port[@address]" (TCP) или
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
	"milter",			/* название */

	SMFI_VERSION,			/* версия API */
	0,				/* флаги */

	/*
	 * Флаги структуры описания:
	 *
	 *	SMFIF_ADDHDRS	- фильтр может добавлять поля заголовка
	 *	SMFIF_CHGHDRS	- фильтр может менять и/или удалять поля
	 *			  заголовка
	 *	SMFIF_CHGBODY	- фильтр можем изменять тело сообщения
	 *	SMFIF_ADDRCPT	- фильтр может добавлять получателей
	 *	SMFIF_DELRCPT	- фильтр может удалять получателей
	 *	SMFIF_QUARANTINE- фильтр может поместить сообщение накарантин
	 */

	mlfi_connect,			/* фильтр подключения */
	mlfi_helo,			/* фильтр HELO */
	mlfi_envfrom,			/* фильтр отправителя */
	mlfi_envrcpt,			/* фильтр получателя */
	mlfi_header,			/* фильтр заголовка */
	mlfi_eoh,			/* фильтр конца заголовка */
	mlfi_body,			/* фильтр тела сообщения */
	mlfi_eom,			/* фильтр конца сообщения */

	mlfi_abort,			/* функция прерывания обработки */
	mlfi_close			/* функция завершения обработки */
};


int
main(int argc, char * const *argv)
{
	if (argc != 2)
		usage();

	/* создать socket фильтра */
	(void)smfi_setconn(argv[1]);

	/* зарегистрировать фильтр */
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
 * Ниже идет описание функций, доступных для использования внутри
 * функций фильтра. Здесь не приводится описания других типов функций.
 * Более подробная информация доступна тут:
 *	http://www.milter.org/milter_api/api.html
 *
 * Функции Milter API, доступные для использования:
 *
 * Функции доступа к данным:
 *
 * char *smfi_getsymval(SMFICTX *ctx, char *symname)
 *	- возвращает значение макроса conf_MILTER_MACROS_имя (NULL если не
 *	  определен)
 *		symname	- имя макроса
 * void *smfi_getpriv(SMFICTX *ctx)
 *	- возвращает внутренние данные фильтра (см. smfi_setpriv)
 *
 * int smfi_setpriv(SMFICTX *ctx, void *data)
 *	- устанавливает внутренние данные фильтра (возвращает MI_SUCCESS
 *	  или MI_FAILURE)
 *		data	- указатель на внутренние данные
 *
 * int smfi_setreply(SMFICTX *ctx, char *rcode, char *xcode, char *message)
 *	- устанавливает код SMTP ошибки для возврата (только 4xx или 5xx,
 *	  возвращает MI_SUCCESS или MI_FAILURE)
 *		rcode	- RFC 821/2821 код
 *		xcode	- 1893/2034 расширеный код (может быть NULL)
 *		message	- сообщение об ошибке
 *
 * int smfi_setmreply(SMFICTX *ctx, char *rcode, char *xcode, ...)
 *	- устанавливает код многострочной SMTP ошибки для возврата
 *	  (только 4xx или 5xx, возвращает MI_SUCCESS или MI_FAILURE)
 *		rcode	- RFC 821/2821 код
 *		xcode	- 1893/2034 расширеный код (может быть NULL)
 *		...	- строки сообщений (до 32)
 *
 * Функции модификации сообщения (возвращают MI_SUCCESS или MI_FAILURE):
 *
 * int smfi_addheader(SMFICTX *ctx, char *headerf, char *headerv)
 *	- добавляет поле заголовка (вызывается только из mlfi_eom,
 *	  требуется флаг SMFIF_ADDHDRS)
 *		headerf	- имя поля заголовка
 *		headerv - значения поля заголовка
 *
 * int smfi_chgheader(SMFICTX *ctx, char *headerf, mi_int32 hdridx,
 *   char *headerv)
 *	- изменяет поле заголовка (вызывается только из mlfi_eom,
 *	  требуется флаг SMFIF_CHGHDRS)
 *		headerf	- имя поля заголовка
 *		hdridx	- номер вхождения поля (начаная с 1); если
 *			  номер больше числа вхождений - добавляется
 *			  новое поле
 *		headerv - значения поля заголовка (NULL - удалить поле)
 *
 * int smfi_insheader(SMFICTX *ctx, mi_int32 hdridx, char *headerf,
 *   char *headerv)
 *	- вставляет поле заголовка (вызывается только из mlfi_eom,
 *	  требуется флаг SMFIF_ADDHDRS)
 *		hdridx	- порядковый номер в списке (0 - в самое начало)
 *		headerf	- имя поля заголовка
 *		headerv - значения поля заголовка
 *
 * int smfi_addrcpt(SMFICTX *ctx, char *rcpt)
 *	- добавляет получателя (вызывается только из mlfi_eom,
 *	  требуется флаг SMFIF_ADDRCPT)
 *		rcpt	- получатель
 *
 * int smfi_delrcpt(SMFICTX *ctx, char *rcpt)
 *	- удаляет получателя (вызывается только из mlfi_eom,
 *	  требуется флаг SMFIF_DELRCPT)
 *		rcpt	- получатель
 *
 * int smfi_replacebody(SMFICTX *ctx, unsigned char *bodyp, int bodylen)
 *	- заменяет тело письма (вызывается только из mlfi_eom, требуется
 *	  флаг SMFIF_CHGBODY)
 *		bodyp	- текущий указатель тела сообщения
 *		bodylen	- длина
 *
 * Функции связи с sendmail (возвращают MI_SUCCESS или MI_FAILURE):
 *
 * int smfi_progress(SMFICTX *ctx)
 *	- сообщает sendmail'у что обработка сообщения еще идет и сбрасывает
 *	  счетчик тайм-аута (вызывается только из mlfi_eom)
 *
 * int smfi_quarantine(SMFICTX *ctx, char *reason)
 *	- помещает сообщение на карантин с указанием причины (вызывается
 *	  только из mlfi_eom, требуется флаг SMFIF_QUARANTINE)
 *		reason	- причина постановки на карантин
 */

/*
 * Фильтр подключения. Вызывается в момент подключения клиента.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *	hostname	- имя хоста клиента или IP адрес в виде [xx.yy.zz.tt]
 *	hostaddr	- структура sockaddr полученная из getpeername или
 *			  NULL если соекдинение инициировано с stdin
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть соединение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 */
static sfsistat
mlfi_connect(SMFICTX *ctx, char *hostname, _SOCK_ADDR *hostaddr)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр HELO. Вызывается после подачи коиманды HELO/EHLO.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *	helohost	- аргумент команды HELO или EHLO
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть соединение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 */
static sfsistat
mlfi_helo(SMFICTX *ctx, char *helohost)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр отправителя. Вызывается после подачи команды MAIL FROM.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *	argv		- массив аргументов команды, завершаемый NULL
 *			  (argv[0] содержит адрес отправителя)
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть соединение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 */
static sfsistat
mlfi_envfrom(SMFICTX *ctx, char **argv)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр получателя. Вызывается после подачи команды RCPT TO.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *	argv		- массив аргументов команды, завершаемый NULL
 *			  (argv[0] содержит адрес получателя)
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть текущего получателя
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку для текущего получателя
 *	SMFIS_DISCARD	- игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки
 */
static sfsistat
mlfi_envrcpt(SMFICTX *ctx, char **argv)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр заголовка. Вызывается после подачи команды RCPT TO.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *	headerf		- название поля заголовка
 *	headerv		- значение поля заголовка (без CR/LF в конце)
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть сообщение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 *	SMFIS_DISCARD	- игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки
 */
static sfsistat
mlfi_header(SMFICTX *ctx, char *headerf, char *headerv)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр конца заголовка. Вызывается после обработки заголовка.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть сообщение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 *	SMFIS_DISCARD	- игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки
 */
static sfsistat
mlfi_eoh(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр тела сообщения. Вызывается один или несколько раз после mkfi_eoh
 * и перед mlfi_eom.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *	bodyp		- указатель на текущий блок тела
 *	len		- длина текущего блока
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть сообщение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 *	SMFIS_DISCARD	- игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки
 */
static sfsistat
mlfi_body(SMFICTX *ctx, unsigned char *bodyp, size_t len)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр конца сообщения. Вызывается восле обработки тела сообщения.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить обработку сообщения
 *	SMFIS_REJECT	- отвергнуть сообщение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 *	SMFIS_DISCARD	- игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки
 */
static sfsistat
mlfi_eom(SMFICTX *ctx)
{
	smfi_addheader(ctx, "X-Shit-Happens", "Yes");

	return (SMFIS_CONTINUE);
}

/*
 * Функция прерывания обработки. Вызывается если обработка сообщения
 * прекращена до завершения и не был возвращен код SMFIS_REJECT,
 * SMFIS_ACCEPT или SMFIS_DISCARD.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить
 */
static sfsistat
mlfi_abort(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * Функция завершения обработки. Вызывается если обработка сообщения
 * завершена.
 *
 * Получаемяе параметры:
 *	ctx		- структура, используемая в вызовах milter
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- продолжить
 *	SMFIS_REJECT	- отвергнуть сообщение
 *	SMFIS_ACCEPT	- принять сообщение без дальнейших проверок
 *	SMFIS_TEMPFAIL	- вернуть временную ошибку
 *			  не возвращая кода ошибки
 */
static sfsistat
mlfi_close(SMFICTX *ctx)
{
	return (SMFIS_ACCEPT);
}
