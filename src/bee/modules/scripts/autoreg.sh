#!/bin/sh

# $RuOBSD$
# new user account registration example

if [ $# -eq 1 ]; then
	PASS=`date|md5|cut -c 1-8`
	EPASS=`echo -n ${PASS}|encrypt`
	groupadd ${1} && \
	/usr/sbin/adduser -batch ${1} ${1} ${1} ${EPASS} -shell nologin -silent > /dev/null && \
	edquota -p test ${1} && \
	echo "000 ${PASS}"
	if [ -z `grep ${1} /etc/ftpchroot` ]; then
		echo ${1} >> /etc/ftpchroot
	fi
else
	echo "incorrect parameters count"
	exit 1
fi
