VERSIONID(`$RuOBSD: sample.mc,v 1.20 2005/10/09 10:34:15 form Exp $')dnl
dnl
OSTYPE(openbsd)dnl
dnl
dnl Настройки безопасности.
dnl
define(`confPRIVACY_FLAGS', `authwarnings,needmailhelo,noexpn,novrfy')dnl
dnl
dnl UUCP - пережиток поршлого. Запрещаем использование UUCP адресов.
dnl
FEATURE(nouucp, `reject')dnl
dnl
dnl Разрешаем использовать /etc/mail/access если файл существует.
dnl
FEATURE(`access_db', `hash -o -T<TMPF> /etc/mail/access')dnl
FEATURE(`blacklist_recipients')dnl
dnl
dnl Включить блокировку по черным спискам DNS.
dnl
dnl FEATURE(`dnsbl', `list.dsbl.org', `Rejected - see http://dsbl.org/faq-listed/')dnl
dnl FEATURE(`dnsbl', `relays.ordb.org', `Rejected - see http://www.ordb.org/')dnl
dnl FEATURE(`dnsbl', `bl.spamcop.net', `"Spam blocked - see: http://spamcop.net/bl.shtml?"$&{client_addr}')dnl
dnl
dnl Разрешаем использовать /etc/mail/local-host-names
dnl
FEATURE(`use_cw_file')dnl
dnl
dnl Разрешаем использовать /etc/mail/mailertable и /etc/mail/virtusertable
dnl если файлы существуют.
dnl
FEATURE(`mailertable', `hash -o /etc/mail/mailertable')dnl
FEATURE(`virtusertable', `hash -o /etc/mail/virtusertable')dnl
dnl
dnl Добавлять имя домена в адрес отправителя даже если пересылка
dnl почты делается внутри хоста.
dnl
FEATURE(always_add_domain)dnl
dnl
dnl Разрешает делать redirect алиасы для сменившихся почтовых ящиков.
dnl
FEATURE(redirect)dnl
dnl
dnl Разрешить форвард в программу только для разрешенных программ.
snl См. man smrhs для подробной информации.
dnl
FEATURE(`smrsh')dnl
dnl
dnl Настройки TLS для защиты SMTP соединения. Для более детальной информации
dnl смотрите starttls(8).
dnl
define(`CERT_DIR', `MAIL_SETTINGS_DIR`'certs')dnl
define(`confCACERT_PATH', `CERT_DIR')dnl
define(`confCACERT', `CERT_DIR/CAcert.pem')dnl
define(`confSERVER_CERT', `CERT_DIR/mycert.pem')dnl
define(`confSERVER_KEY', `CERT_DIR/mykey.pem')dnl
define(`confCLIENT_CERT', `CERT_DIR/mycert.pem')dnl
define(`confCLIENT_KEY', `CERT_DIR/mykey.pem')dnl
dnl
dnl Настройки SMTP авторизации (sendmail должен быть собран с cyrus-sasl2).
dnl
dnl Запретить использовать метод авторизации PLAIN по незащищенному
dnl соединению.
dnl
define(`confAUTH_OPTIONS', `p')dnl
dnl
dnl Список допустимых методов авторизации.
dnl
define(`confAUTH_MECHANISMS', `DIGEST-MD5 CRAM-MD5 PLAIN LOGIN')dnl
TRUST_AUTH_MECH(`DIGEST-MD5 CRAM-MD5 PLAIN LOGIN')dnl
dnl
dnl Использовать clamav-milter для проверки почты на наличие вирусов
dnl
dnl INPUT_MAIL_FILTER(`clamav', `S=inet:1025@127.0.0.1, F=T, T=S:4m;R:4m')dnl
dnl
dnl Использовать mail.buhal вместо mail.local для доставки почты
dnl в Maildir пользователей
dnl
dnl define(`LOCAL_MAILER_PATH', `/usr/libexec/mail.buhal')dnl
dnl MODIFY_MAILER_FLAGS(`LOCAL', `-m')dnl
dnl
dnl Создать выделенный SSL порт 465 (чистый SSL без STARTTLS)
dnl
dnl DAEMON_OPTIONS(`Name=MTA')dnl
dnl DAEMON_OPTIONS(`Port=465, Name=MTA-SSL, M=s')dnl
dnl
dnl Список почтовых агентов
dnl
MAILER(local)dnl
MAILER(smtp)dnl
dnl
dnl
LOCAL_CONFIG
#
# Список доменов, появление которых в "Reveived:" с большой вероятностью
# указывает на спам. См. правило "CheckReceived" ниже.
#
# ВНИМАНИЕ: проверяется не полное совпадение домена/поддомена, а
# наличие подстроки в поле `Received:'. Возможны незапланированные
# срабатывания (например domain.com сработает как на super.domain.com
# так и на domain.computer.org). Это правило следует использовать только
# в крайнем случае если идет много спама через нормальные сети (например
# через MX'ы или forward).
#
#FS-o /etc/mail/spam-domains

#
# Регулярное выражение для блокировки возможного спама:
#
# - адреса, содержащие более двух минусов в имени
# - адреса с тремя группами цифр, разделенными точками
# - адреса с минусом между цифрами
# - адреса, содержащие четыре или более цифр подряд
# - адреса с доменным именем выше 4 уровня
# - адреса, содержащие в имени "dsl", "pppoe", "dial", "dynamic", "dhcp"
#
# См. Ниже правило "Basic_check_relay".
#
#Kcheckhost regex -a<MATCH> (.+-.+-.+|[0-9]+\.[0-9]+\.[0-9]+\.|[0-9]+-[0-9]+|[0-9]{4}|[^.]+\.[^.]+\.[^.]+\.[^.]+\.|dsl|pppoe|dial|dynamic|dhcp)

LOCAL_RULESETS
#
# Проверка заголовка
#
HMessage-Id: $>CheckMessageId
#HReceived: $>+CheckReceived

#
# Не пропускать письма с неправильным форматом Message-Id
#
SCheckMessageId
R< $+ @ $+ >		$@ OK
R$*			$#error $: 553 Header error

#
# Проверить "Received:" на совпадение со списком доменов из файла
# `/etc/spam-domains'
#
#SCheckReceived
#R$* $=S $*		$#error $@ 5.7.1 $: "550 Access denied"

#
# Не пропускать письма, поступившие от серверов без имени или с именем,
# не соответствующим IP адресу. Во избежание ложного срабатывания данных
# правил рекомендуется локальные сети (и другие адреса/сети, имеющие право
# отправлять почту через данный сервер) прописать в файле `/etc/mail/access'
# в виде:
#
# Connect:192.168	OK
# Connect:81.1.212.10	OK
#
#SBasic_check_relay
#R$*			$: < $&{client_resolve} >
#R< TEMP >		$#error $@ 4.7.1 $: "450 Access temporarily denied. Cannot resolve PTR record for " $&{client_addr}
#R< FAIL >		$#error $@ 5.7.1 $: "550 Access denied. IP name lookup failed " $&{client_addr}
#R< FORGED >		$#error $@ 5.7.1 $: "550 Access denied. IP name possibly forged " $&{client_addr}
#
# Проверить имя хоста по регулярному выражению выше
#
#R$*			$: $&{client_name}
#R$*			$: $(checkhost $1 $)
#R< MATCH >		$#error $@ 5.7.1 $: "550 Access denied"
