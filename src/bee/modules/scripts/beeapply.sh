#!/bin/sh

# $RuOBSD: beeapply.sh,v 1.5 2002/10/31 20:41:34 shadow Exp $

# IPF rules set-up (local)
#ipf -Fa -f /var/bee/ipf.rules.effective

# PF rules set-up (local)
#pfctl -R /var/bee/pf.conf.effective 

# OpenBSD 3.1  PF rules set-up (remote)
#ssh root@orion.oganer.net "pfctl -R /var/bee/pf.conf.effective" > /dev/null 

# OpenBSD 3.2+ PF rules set-up (remote)
#ssh root@orion.oganer.net "pfctl -R /var/bee/pf.conf.effective" > /dev/null 



