#!/bin/sh

# $RuOBSD: autoreg.sh,v 1.1 2001/09/18 05:36:13 tm Exp $
# new user account registration example

if [ $# -eq 1 ]; then
	PASS=`date|md5|cut -c 1-8`
	EPASS=`echo -n ${PASS}|encrypt` && \
	groupadd ${1} && \
	/usr/sbin/useradd -m -p ${EPASS} -s nologin -g ${1} ${1}> /dev/null && \
	edquota -p test ${1} && \
	echo "000 ${PASS}"
	if [ -z `grep ${1} /etc/ftpchroot` ]; then
		echo ${1} >> /etc/ftpchroot
	fi
else
	echo "incorrect parameters count"
	exit 1
fi
