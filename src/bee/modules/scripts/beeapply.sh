#!/bin/sh

# $RuOBSD: beeapply.sh,v 1.3 2002/06/03 17:38:50 shadow Exp $

# IPF rules set-up (local)
#ipf -Fa -f /var/bee/ipf.rules.effective

# PF rules set-up (local)
#pfctl -R /var/bee/pf.conf.effective 

# PF rules set-up (remote)
ssh root@orion.oganer.net "pfctl -R /var/bee/pf.conf.effective" > /dev/null 

ssh root@vega.oganer.net "pfctl -R /var/bee/pf.conf.effective" > /dev/null 


