#!/bin/sh

# pf (local) & tun0
#/usr/local/src/beeipf -s /etc/pf.conf -t /var/bee/pf.conf.effective -l log 

# pf (remote) 
#cat /var/bee/allowed.inet | ssh root@orion.oganer.net "/usr/local/bin/beeipf -s /etc/pf.conf -t /var/bee/pf.conf.effective -f - -l log -R -i rl0"


