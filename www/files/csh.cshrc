# $RuOBSD$

set notify
if ($?tcsh) then
	set correct=cmd
	set prompt='%m:%~%# '
endif

if (-x /usr/local/bin/gls && -x /usr/local/bin/gdircolors) then
	if (-r /etc/DIR_COLORS) then
		eval `/usr/local/bin/gdircolors -c /etc/DIR_COLORS`
	else
		eval `/usr/local/bin/gdircolors -c`
	endif
	if ("$LS_COLORS" != "") then
		alias ls /usr/local/bin/gls --color=tty
	else
		unsetenv LS_COLORS
		alias ls /bin/ls -F
	endif
else
	alias ls /bin/ls -F
endif

alias dir ls
alias vdir ls -l
alias d dir
alias v vdir

alias r traceroute
alias lo locate
alias j jobs -l
alias out-of-date /usr/ports/infrastructure/build/out-of-date
