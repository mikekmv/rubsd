#!/bin/sh
# $RuOBSD: intractl.sh,v 1.4 2002/06/01 19:13:56 shadow Exp $

# script to control Cisco's Catalyst 1900 series switch ports via SNMP

# ucd-snmp required

# Script deactivator (Comment to activate script)
exit

CONFIG_FILE=/etc/bee/intra.conf
SNMPWALK=/usr/local/bin/snmpwalk
SNMPSET=/usr/local/bin/snmpset

RES_SEPARATOR=":"
AWK=/usr/bin/awk

ALLOW=/var/bee/allowed.intra
DENY=/var/bee/disallowed.intra

# Uncomment if no periodical "update" execution
#/usr/local/bin/bee -u

if [ $# -eq 1 ]; then
	CONFIG_FILE=$1
fi

if [ ! -r ${CONFIG_FILE} ]; then
	echo "can't read ${CONFIG_FILE}, aborting"
	exit 1
fi

if [ -r ${ALLOW} ]; then
	AWK_VARS=${AWK_VARS}" -v ALLOW=${ALLOW}"
fi

if [ -r ${DENY} ]; then
	AWK_VARS=${AWK_VARS}" -v DENY=${DENY}"
fi

AWK_VARS=${AWK_VARS}" -v CONFIG=${CONFIG_FILE}"

/bin/echo "" | ${AWK} ${AWK_VARS} '

function find_device(table, id) {
	for (idx in table) {
		if (index(table[idx], id))
			return idx
	}
	return 0;
}

BEGIN {
	while (getline < CONFIG) {
		if (length($0) && !index($0, "#") && NF == 4) {
			id_table[++c]=$1
			host_table[c]=$2
			set_table[c]=$3
			if_table[c]=$4
		}
	}

	if (length(DENY) || length(ALLOW)) {
		if (length(DENY)) {
			while (getline < DENY) {
				split($0, tmp, ":")
				if ((idx = find_device(id_table, tmp[1]))) {
					system("/usr/local/bin/snmpset "host_table[idx]" "set_table[idx]" "if_table[idx]"."tmp[2]" i 2");
				}
				else
					print "WARNING!, device", tmp[1], "not found in", CONFIG

			}	
		}
		if (length(ALLOW)) {
			while (getline < ALLOW) {
				split ($0, tmp, ":")
				if ((idx = find_device(id_table, tmp[1]))) {
					system("/usr/local/bin/snmpset "host_table[idx]" "set_table[idx]" "if_table[idx]"."tmp[2]" i 1")
				}
				else
					print "WARNING!, device", tmp[1], "not found in", CONFIG
			}
		}
	}
}
	
{}

END {
}'
