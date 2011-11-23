# $RuOBSD: csh.cshrc,v 1.6 2006/03/23 12:41:43 form Exp $

set notify
if ($?tcsh) then
	set correct=cmd
	set prompt='%m:%~%# '
	if ($?TERM) then
		if ("$TERM" == "vt52" || "$TERM" == "dec vt52") then
			bindkey "^[H" beginning-of-line
			bindkey "^[Q" run-help
			bindkey "^[R" kill-line
		else
			bindkey "^[[1~" beginning-of-line
			bindkey "^[[7~" beginning-of-line
			bindkey "^[[4~" end-of-line
			bindkey "^[[8~" end-of-line
			bindkey "^[[3~" delete-char
		endif
	endif
	alias helpcommand man
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
else if (-x /usr/local/bin/colorls) then
	setenv LSCOLORS "ExGxFxfxCxegedacagacad"
	alias ls /usr/local/bin/colorls -G
else
	alias ls /bin/ls -F
endif

if (-x /usr/local/bin/xchat) alias ircify sed -e \''s,^.*$,%C03&,'\'
if (-x /usr/ports/infrastructure/bin/out-of-date) then
	alias out-of-date /usr/ports/infrastructure/bin/out-of-date
endif

alias dir ls
alias vdir ls -l
alias d dir
alias v vdir

alias r traceroute
alias lo locate
alias j jobs -l
