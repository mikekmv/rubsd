/*
 * Шаблон для написания фильтров к sendmail с использованием Milter API.
 * Он  не  выполняет  никаких  действий  и содержит только определения и
 * описания функций.
 *
 * Запуск фильтра: milter socket
 * где socket может быть указан в виде "inet:port[@address]" (TCP) или
 * "local:path" (UNIX).
 *
 * $RuOBSD: milter.c,v 1.4 2005/10/08 05:33:35 form Exp $
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
static sfsistat mlfi_unknown(SMFICTX *, const char *);
static sfsistat mlfi_data(SMFICTX *);
static sfsistat mlfi_negotiate(SMFICTX *, u_long, u_long, u_long, u_long,
    u_long *, u_long *, u_long *, u_long *);


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
	 *	SMFIF_CHGBODY	- фильтр может изменять тело сообщения
	 *	SMFIF_ADDRCPT	- фильтр может добавлять получателей
	 *	SMFIF_ADDRCPT_PAR - фильтр может добавлять получателей с ESMTP
	 *			  аргументами
	 *	SMFIF_DELRCPT	- фильтр может удалять получателей
	 *	SMFIF_QUARANTINE- фильтр может поместить сообщение на карантин
	 *	SMFIF_CHGFROM	- фильтр может менять отправителя
	 *	SMFIF_SETSYMLIST - фильтр может вызывать smfi_setsymlist
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
	mlfi_close,			/* функция завершения обработки */
	mlfi_unknown,			/* функция неподдерживаемой команды */
	mlfi_data,			/* функция команды DATA */
	mlfi_negotiate,			/* установочная функция */
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

	/* переключиться в фоновй режим */
	if (daemon(0, 0) < 0)
		err(EX_OSERR, NULL);

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
 * int smfi_chgfrom(SMFICTX *ctx, const char *mail, char *args)
 *	- изменяет отправителя сообщения (вызывается только из mkfi_eom,
 *	  требуется флаг SMFIF_CHGFROM)
 *		mail	- новый адрес отправителя
 *		args	- ESMTP аргументы
 *
 * int smfi_addrcpt(SMFICTX *ctx, char *rcpt)
 *	- добавляет получателя (вызывается только из mlfi_eom,
 *	  требуется флаг SMFIF_ADDRCPT)
 *		rcpt	- получатель
 *
 * int smfi_addrcpt_par(SMFICTX *ctx, char *rcpt, char *args)
 *	- добавляет получателя с ESMTP аргументами (вызывается только
 *	  из mlfi_eom, требуется флаг SMFIF_ADDRCPT_PAR)
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
 *
 * Разные функции (возвращают MI_SUCCESS или MI_FAILURE):
 *
 * int smfi_version(unsigned int *pmajor, unsigned int *pminor,
 *     unsigned int *ppl)
 *	- возвращает версию Milter API
 *		pmajor	- major версия
 *		pminor	- minor версия
 *		ppl	- patch level
 *
 * int smfi_setsymlist(SMFICTX *ctx, int stage, char *macros)
 *	- устанавливает список макросов, которые фильтр планирует
 *	  проверять (вызывается только из mlfi_negotiate)
 *		stage	- стадия, на которой фильтр планирует проверять
 *			  значение макросов: SMFIM_HELO, SMFIM_MAIL,
 *			  SMFIM_RCPT, SMFIM_DATA, SMFIM_EOH, SMFIM_EOM
 *		macros	- список макросов, разделенный пробелами
 */

/*
 * Фильтр подключения. Вызывается в момент подключения клиента.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *	hostname	- Имя хоста клиента или IP адрес в виде [xx.yy.zz.tt].
 *	hostaddr	- Структура sockaddr полученная из getpeername или
 *			  NULL если соединение инициировано с stdin.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть соединение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
 */
static sfsistat
mlfi_connect(SMFICTX *ctx, char *hostname, _SOCK_ADDR *hostaddr)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр HELO. Вызывается после подачи команды HELO/EHLO.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *	helohost	- Аргумент команды HELO или EHLO.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть соединение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
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
 *	ctx		- Структура, используемая в вызовах milter.
 *	argv		- Массив аргументов команды, завершаемый NULL
 *			  (argv[0] содержит адрес отправителя).
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть соединение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
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
 *	ctx		- Структура, используемая в вызовах milter.
 *	argv		- Массив аргументов команды, завершаемый NULL
 *			  (argv[0] содержит адрес получателя).
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть текущего получателя.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку для текущего получателя.
 *	SMFIS_DISCARD	- Игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
 */
static sfsistat
mlfi_envrcpt(SMFICTX *ctx, char **argv)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр заголовка. Вызывается для каждого поля заголовка.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *	headerf		- Название поля заголовка.
 *	headerv		- Значение поля заголовка (без CR/LF в конце).
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_DISCARD	- Игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
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
 *	ctx		- Структура, используемая в вызовах milter
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_DISCARD	- Игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
 */
static sfsistat
mlfi_eoh(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * Фильтр тела сообщения. Вызывается один или несколько раз после mlfi_eoh
 * и перед mlfi_eom.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *	bodyp		- Указатель на текущий блок тела.
 *	len		- Длина текущего блока.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_DISCARD	- Игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки.
 *	SMFIS_SKIP	- Не вызывать в дальнейшем mlfi_body для текущего
 *			  сообщения.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
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
 *	ctx		- Структура, используемая в вызовах milter.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_DISCARD	- Игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки.
 */
static sfsistat
mlfi_eom(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * Функция прерывания обработки. Вызывается если обработка сообщения
 * прекращена до завершения и не был возвращен код SMFIS_REJECT,
 * SMFIS_ACCEPT или SMFIS_DISCARD.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить.
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
 *	ctx		- Структура, используемая в вызовах milter.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 */
static sfsistat
mlfi_close(SMFICTX *ctx)
{
	return (SMFIS_ACCEPT);
}

/*
 * Функция неподдерживаемой команды. Вызывается при выдаче
 * клиентом неизвестной или неподдерживаемой SMTP команды.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *	arg		- SMTP команда с аргументами.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
 */
static sfsistat
mlfi_unknown(SMFICTX *ctx, const char *arg)
{
	return (SMFIS_CONTINUE);
}

/*
 * Функция команды DATA. Вызывается при выдаче клиентом SMTP
 * команды DATA.
 *
 * Получаемяе параметры:
 *	ctx		- Cтруктура, используемая в вызовах milter.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Отвергнуть сообщение.
 *	SMFIS_ACCEPT	- Принять сообщение без дальнейших проверок.
 *	SMFIS_TEMPFAIL	- Вернуть временную ошибку.
 *	SMFIS_DISCARD	- Игнорировать сообщение без дальнейшей проверки,
 *			  не возвращая кода ошибки.
 *	SMFIS_NOREPLY	- Не возвращать состояние в MTA. См. mlfi_negotiate.
 */
static sfsistat
mlfi_data(SMFICTX *ctx)
{
	return (SMFIS_CONTINUE);
}

/*
 * Установочная функция. Вызывается в начале SMTP сессии для
 * проверки/установки опций milter.
 *
 * Получаемяе параметры:
 *	ctx		- Структура, используемая в вызовах milter.
 *	f0		- Действия, предполагаемые MTA. Это поле соответствует
 *			  полю флагов структуры smfiDesc.
 *	f1		- Шаги, предполагаемые MTA. См. ниже.
 *	f2		- Зарезервированно.
 *	f3		- Зарезервированно.
 *	pf0		- Действия, запрашиваемые фильтром.
 *	pf1		- Шаги, запрашиваемые фильтром.
 *	pf2		- Зарезервированно.
 *	pf3		- Зарезервированно.
 *
 * Маска шагов (f1, pf1):
 *	SMFIP_RCPT_REJ	- MTA должен передавать RCPT команды в фильтр
 *			  даже если команда была отвергнута по какой-либо
 *			  причине (кроме синтаксической ошибки). Фильтр
 *			  должен проверять макрос {rcpt_mailer}: если
 *			  содержит "error", значит адрес был отвергнут.
 *			  В этом случае макросы {rcpt_host} и {rcpt_addr}
 *			  содержат код ошибки и сообщение об ошибке
 *			  соответственно.
 *	SMFIP_SKIP	- Признак того, что MTA понимает код возврата
 *			  SMFIS_SKIP.
 *	SMFIP_NR_CONN	- MTA поддерживает SMFIS_NOREPLY в mlfi_connect.
 *	SMFIP_NR_HELO	- MTA поддерживает SMFIS_NOREPLY в mlfi_helo.
 *	SMFIP_NR_MAIL	- MTA поддерживает SMFIS_NOREPLY в mlfi_envfrom.
 *	SMFIP_NR_RCPT	- MTA поддерживает SMFIS_NOREPLY в mlfi_envrcpt.
 *	SMFIP_NR_DATA	- MTA поддерживает SMFIS_NOREPLY в mlfi_data.
 *	SMFIP_NR_UNKN	- MTA поддерживает SMFIS_NOREPLY в mlfi_unknown.
 *	SMFIP_NR_EOH	- MTA поддерживает SMFIS_NOREPLY в mlfi_eoh.
 *	SMFIP_NR_BODY	- MTA поддерживает SMFIS_NOREPLY в mlfi_body.
 *	SMFIP_NR_HDR	- MTA поддерживает SMFIS_NOREPLY в mlfi_header.
 *	SMFIP_HDR_LEADSPC - MTA может передавать значение поля заголовка
 *			  с пробелами в начале.
 *	SMFIP_NOCONNECT	- Запретить MTA вызывать mlfi_connect.
 *	SMFIP_NOHELO	- Запретить MTA вызывать mlfi_help.
 *	SMFIP_NOMAIL	- Запретить MTA вызывать mlfi_envfrom.
 *	SMFIP_NORCPT	- Запретить MTA вызывать mlfi_envrcpt.
 *	SMFIP_NOBODY	- Запретить MTA вызывать mlfi_body.
 *	SMFIP_NOHDRS	- Запретить MTA вызывать mlfi_header.
 *	SMFIP_NOEOH	- Запретить MTA вызывать mlfi_eoh.
 *	SMFIP_NOUNKNOWN	- Запретить MTA вызывать mlfi_unknown.
 *	SMFIP_NODATA	- Запретить MTA вызывать mlfi_data.
 *
 * Коды возврата:
 *	SMFIS_CONTINUE	- Продолжить обработку сообщения.
 *	SMFIS_REJECT	- Ошибка инициализации. Запрещает дальнейшее
 *			  использоваение фильтра для текущей сессии.
 *	SMFIS_ALL_OPTS	- Продолжить выполнение без изменения опций.
 */
static sfsistat
mlfi_negotiate(SMFICTX *ctx, u_long f0, u_long f1, u_long f2, u_long f3,
    u_long *pf0, u_long *pf1, u_long *pf2, u_long *pf3)
{
	return (SMFIS_ALL_OPTS);
}
