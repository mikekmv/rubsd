#!/bin/sh
#
# $RuOBSD$

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
