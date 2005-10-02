# $RuOBSD: csh.cshrc,v 1.2 2005/05/05 19:38:02 form Exp $

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

if (-x /usr/local/bin/xchat) alias ircify sed -e \''s,^.*$,%C03&,'\'
if (-x /usr/ports/infrastructure/build/out-of-date) then
	alias out-of-date /usr/ports/infrastructure/build/out-of-date
endif

alias dir ls
alias vdir ls -l
alias d dir
alias v vdir

alias r traceroute
alias lo locate
alias j jobs -l
