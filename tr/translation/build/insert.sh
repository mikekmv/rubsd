#!/bin/sh

if [ $# -ne 2 ]; then
	exit 1;
fi

FILE="$1"
TAG="$2"

BASENAME=`basename ${FILE}`
TMPFILE=`mktemp /tmp/${BASENAME}.XXXXXX` || exit 1
trap 'rm -f ${TMPFILE}' 0 1 15

# insert head
sed -e "/<!-- ${TAG}::START -->/q" ${FILE} > ${TMPFILE}

# insert from stdin
cat >> ${TMPFILE}

# insert tail
sed -e "/<!-- ${TAG}::END -->/,\$!d" ${FILE} >> ${TMPFILE}

mv -f ${TMPFILE} ${FILE}
chmod a+r ${FILE}
