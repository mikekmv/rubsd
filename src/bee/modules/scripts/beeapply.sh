#!/bin/sh
# $RuOBSD: beeapply.sh,v 1.7 2005/04/05 08:45:54 shadow Exp $


# generated PF rules set-up (local)
#pfctl -f /var/bee/pf.conf.effective 

# generated PF rules set-up (remote)
#REMOTE=root@orion.oganer.net
#ssh ${REMOTE} "pfctl -f /var/bee/pf.conf.effective" > /dev/null 

# table version (no rule generatior used, local)
#pfctl -q -t hBeeAllowed -T replace -f /var/bee/allowed.inet

# table version (no rule generatior used, remote)
#REMOTE=root@orion.oganer.net
#scp -q -C /var/bee/allowed.inet ${REMOTE}:/var/bee
#ssh ${REMOTE} "pfctl -q -t hBeeAllowed -T replace -f /var/bee/allowed.inet"

