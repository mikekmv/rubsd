#!/bin/sh
# $RuOBSD: beetraff-cnupm.sh,v 1.1 2005/07/30 22:43:13 shadow Exp $

#
# for Local firewall
#

#TEMPFILE=`/usr/bin/mktemp /var/bee/beetraff.XXXXXXXX` || exit 1
#/usr/local/bin/bee-cnupmdump.sh > ${TEMPFILE}
#
#/usr/local/bin/beetraff -u -f ${TEMPFILE} -n 192.168.0.0/16 \
#       -n 172.16.0.0/12 -n 10.0.0.0/8
#
#rm -rf ${TEMPFILE}

#
# for Remote firewall:
#

#REMOTE=orion.oganer.net
#
#TEMPFILE=`/usr/bin/mktemp /var/bee/beetraff.XXXXXXXX` || exit 1
#/usr/bin/ssh -C ${REMOTE} "/usr/local/bin/bee-cnupmdump.sh > ${TEMPFILE} && cat ${TEMPFILE} && rm -f ${TEMPFILE}" > ${TEMPFILE}
#
#/usr/local/bin/beetraff -u -f ${TEMPFILE} -n 192.168.0.0/16 \
#       -n 172.16.0.0/12     -n 10.0.0.0/8
#
#rm -rf ${TEMPFILE}

