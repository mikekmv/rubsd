#!/bin/sh
# $RuOBSD: beeapply.sh,v 1.6 2004/05/09 19:38:49 shadow Exp $


# generated PF rules set-up (local)
#pfctl -f /var/bee/pf.conf.effective 

# generated PF rules set-up (remote)
#ssh root@orion.oganer.net "pfctl -f /var/bee/pf.conf.effective" > /dev/null 

# table version (no rule generatior used, local)
#pfctl -q -t hBeeAllowed -T replace -f /var/bee/allowed.inet

# table version (no rule generatior used, remote)
#scp -q -C /var/bee/allowed.inet root@orion.oganer.net:/var/bee
#ssh root@orion.oganer.net "pfctl -q -t hBeeAllowed -T replace -f /var/bee/allowed.inet"

