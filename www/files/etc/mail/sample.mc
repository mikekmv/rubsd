VERSIONID(`$RuOBSD: sample.mc,v 1.2 2004/11/27 20:02:21 form Exp $')dnl
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
TRUST_AUTH_MECH(`GSSAPI DIGEST-MD5 CRAM-MD5 PLAIN')dnl
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
dnl Список почтовых агентов
dnl
MAILER(local)dnl
MAILER(smtp)dnl
dnl
dnl Требовать правильного формата Message-Id в заголовке письма.
dnl
LOCAL_RULESETS
HMessage-Id: $>CheckMessageId

SCheckMessageId
R< $+ @ $+ >		$@ OK
R$*			$#error $: 553 Header Error
