# $RuOBSD$

CONFIG=		pdp11
MSACONFIG=	submit

ALIASES=	aliases
DBASES=		access virtusertable mailertable
FILES=		relay-domains local-host-names

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
sendmail.cf:	${CONFIG}.mc
	/usr/bin/m4 -D_CF_DIR_=${CFDIR}/ ${CFDIR}/m4/cf.m4 ${CONFIG}.mc > $@
.endif

.if defined(MSACONFIG) && ${MSACONFIG} != ""
submit.cf:	${MSACONFIG}.mc
	/usr/bin/m4 -D_CF_DIR_=${CFDIR}/ ${CFDIR}/m4/cf.m4 ${MSACONFIG}.mc > $@
.endif
