# $RuOBSD$
#
# Данный файл упрощает пересборку конфигов sendmail, его баз данных;
# отслеживает изменения в файлах, используемых в работе и при необходимости
# перезапускает активный sendmail.
#
# Для использования данного Makefile достаточно указать список используемых
# файлов ниже, и в дяльнейшем после изменения какого-либо из них достаточно
# просто подать команду `make' чтобы изменения вступили в силу.

# Исходники конфигов для создания sendmail.cf и submit.cf
#
CONFIG=		sample.mc
#MSACONFIG=	submit.mc

# Список файлов алиасов, баз данных и текстовых файлов, используемых
# в работе
#
ALIASES=	aliases
#DBASES=	access virtusertable mailertable
FILES=		relay-domains local-host-names

# Строки ниже этой не требуется редактировать
#
CFDIR=		/usr/share/sendmail
PIDFILE=	/var/run/sendmail.pid

ALIASFILES=	${ALIASES:=.db}
DBASEFILES=	${DBASES:=.db}

.if defined(CONFIG) && ${CONFIG} != ""
CONFIGFILES=	sendmail.cf
.endif
.if defined(MSACONFIG) && ${MSACONFIG} != ""
CONFIGFILES+=	submit.cf
.endif

all:		${PIDFILE}

${PIDFILE}:	${CONFIGFILES} ${DBASEFILES} ${ALIASFILES} ${FILES}
.if exists(${PIDFILE})
	/bin/kill -s HUP `/usr/bin/head -1 ${PIDFILE}`
.endif

.if defined(ALIASES) && ${ALIASES} != ""
${ALIASFILES}:	${ALIASES}
	/usr/bin/newaliases
.endif

.for f in ${DBASES}
${f}.db:	${f}
	/usr/sbin/makemap hash ${f} < ${f}
.endfor

.if defined(CONFIG) && ${CONFIG} != ""
sendmail.cf:	${CONFIG}
	/usr/bin/m4 -D_CF_DIR_=${CFDIR}/ ${CFDIR}/m4/cf.m4 ${CONFIG} > $@
.endif

.if defined(MSACONFIG) && ${MSACONFIG} != ""
submit.cf:	${MSACONFIG}
	/usr/bin/m4 -D_CF_DIR_=${CFDIR}/ ${CFDIR}/m4/cf.m4 ${MSACONFIG} > $@
.endif
