#!/bin/sh
#
# $RuOBSD$

SRC_PATH=`pwd`

echo -n "Enter the path to the system sources [/sys]? "
read SYS_PATH

[ -z "${SYS_PATH}" ] && SYS_PATH=/sys

if [ -f ${SYS_PATH}/dev/ic/sbni.c ]; then
	echo "SBNI 12-xx drivers already installed"
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

cp ${SRC_PATH}/if_sbni_isa.c ${SYS_PATH}/dev/isa \
    && cp ${SRC_PATH}/if_sbni_pci.c ${SYS_PATH}/dev/pci \
    && cp ${SRC_PATH}/sbni.c ${SYS_PATH}/dev/ic \
    && cp ${SRC_PATH}/sbnivar.h ${SYS_PATH}/dev/ic \
    && cp ${SRC_PATH}/sbnireg.h ${SYS_PATH}/dev/ic \
    && patch -p0 < ${SRC_PATH}/patches/patch-conf_files \
    && patch -p0 < ${SRC_PATH}/patches/patch-dev_isa_files_isa \
    && patch -p0 < ${SRC_PATH}/patches/patch-dev_pci_files_pci

if [ $? = "0" ]; then
	echo "SBNI 12-xx driver succesfully installed"
else
	echo "Error installing SBNI 12-xx driver"
fi
