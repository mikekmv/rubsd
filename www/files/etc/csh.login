# $RuOBSD: csh.login,v 1.1 2004/11/01 04:53:46 form Exp $

setenv PATH /bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:/sbin:/usr/sbin:/usr/local/sbin
setenv BLOCKSIZE k
setenv PAGER /usr/bin/less
setenv LESSCHARSET koi8-r

if (-x /usr/local/bin/lesspipe) setenv LESSOPEN "| /usr/local/bin/lesspipe %s"
if (-x /usr/local/bin/joe) setenv EDITOR /usr/local/bin/joe
