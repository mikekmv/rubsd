# $RuOBSD$

setenv PATH /bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin:/sbin:/usr/sbin:/usr/local/sbin
setenv BLOCKSIZE k
setenv PAGER /usr/bin/less
setenv LESSCHARSET koi8-r

if (-x /usr/local/bin/lesspipe) setenv LESSOPEN "| /usr/local/bin/lesspipe %s"
if (-x /usr/local/bin/joe) setenv EDITOR /usr/local/bin/joe
