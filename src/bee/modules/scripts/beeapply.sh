#!/bin/sh

# $RuOBSD: beeapply.sh,v 1.4 2002/06/04 15:58:59 shadow Exp $

# IPF rules set-up (local)
#ipf -Fa -f /var/bee/ipf.rules.effective

# PF rules set-up (local)
#pfctl -R /var/bee/pf.conf.effective 

# PF rules set-up (remote)
#ssh root@orion.oganer.net "pfctl -R /var/bee/pf.conf.effective" > /dev/null 



