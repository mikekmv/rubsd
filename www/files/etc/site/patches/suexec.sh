#!/bin/sh
#
# $RuOBSD: suexec.sh,v 1.1 2003/09/05 07:02:02 form Exp $

edscript()
{
	cat << EOF
1
EOF
	if [ -n "$1" ]; then
		cat << EOF
/^suexec_docroot.*=/
c
suexec_docroot  = $1
.
EOF
	fi
	cat << EOF
/-DLOG_EXEC=/
i
			-DLOGIN_CAP \\
.
w
q
EOF
}

edscript $1 | ed Makefile >/dev/null 2>&1
