#!/bin/sh
# $RuOBSD$

IFACE="pflog0"

# force flush to dumpfile
pkill -HUP cnupm

# give cnupm some time
sleep 2

# output text dump for bee
/usr/local/sbin/cnupmstat -BEn ${IFACE}

# remove dumpfile
rm -rf /var/cnupm/cnupm-${IFACE}.dump
