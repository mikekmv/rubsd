#!/bin/sh

if [ "x${2}x" = "xx" ]; then
   DF=`date +%d.%m.%Y`".00.00.00"
else
   DF="${2}"
fi

if [ "x${3}x" = "xx" ]; then
   DN="1"
else
   DN="${3}"
fi

/usr/local/bin/beerep -F ${DF} -n ${DN} -a ${1} -d -h -c 'border=0 class="txt8" cellspacing=1 cellpadding=4' -C 'class="b1c"' -H 'class="b2c" align=center' -t DTICSPH -R

