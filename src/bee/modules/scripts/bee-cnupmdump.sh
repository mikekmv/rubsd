#!/bin/sh
# $RuOBSD: bee-cnupmdump.sh,v 1.1 2005/07/30 22:43:13 shadow Exp $

IFACE="pflog0"

# force flush to dumpfile
pkill -HUP cnupm

# give cnupm some time
sleep 2

# output text dump for bee
/usr/local/sbin/cnupmstat -En ${IFACE}

# remove dumpfile
rm -rf /var/cnupm/cnupm-${IFACE}.dump
