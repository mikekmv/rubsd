#!/bin/sh
#
# $RuOBSD$

SRC_PATH=`pwd`

echo -n "Enter the path to the system sources [/sys]? "
read SYS_PATH

[ -z "${SYS_PATH}" ] && SYS_PATH=/sys

if [ ! -f ${SYS_PATH}/dev/ic/sbni.c ]; then
	echo "SBNI 12-xx drivers not installed"
	exit 1
fi

if cd ${SYS_PATH} 2>/dev/null; then
	if [ ! -w `pwd`/. ]; then
		echo "Can't write to `pwd` (Permission denied?)"
		exit 1
	fi
else
	echo "Can't chdir to ${SYS_PATH}"
	exit 1
fi

rm -f ${SYS_PATH}/dev/isa/if_sbni_isa.c cp ${SYS_PATH}/dev/pci/if_sbni_pci.c \
    && rm -f ${SYS_PATH}/dev/ic/sbni.c ${SYS_PATH}/dev/ic/sbnivar.h \
    && rm -f ${SYS_PATH}/dev/ic/sbnireg.h \
    && mv -f ${SYS_PATH}/conf/files.orig ${SYS_PATH}/conf/files \
    && mv -f ${SYS_PATH}/dev/isa/files.isa.orig ${SYS_PATH}/dev/isa/files.isa \
    && mv -f ${SYS_PATH}/dev/pci/files.pci.orig ${SYS_PATH}/dev/pci/files.pci

if [ $? = "0" ]; then
	echo "SBNI 12-xx driver succesfully deinstalled"
else
	echo "Error deinstalling SBNI 12-xx driver"
fi
