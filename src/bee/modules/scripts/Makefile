# $RuOBSD: Makefile,v 1.10 2005/04/22 14:47:36 shadow Exp $

SCRIPTS = beeapply.sh newlogin.sh beetraff-ipstatd.sh beepfrules.sh beetraff-cnupm.sh bee-cnupmdump.sh
SBIN    = autoreg.sh intractl.sh

clean:
all:
install: 
	@echo ">>> Installing scripts  to /usr/local/bin <<<"
	@for entry in ${SCRIPTS}; do \
		(echo ">>> $${entry}"; \
                 install -c -o root -g bin -m 550 $${entry} \
			/usr/local/bin/); \
        done;
	@echo ">>> Installing scripts to /usr/local/sbin <<<"
	@for entry in ${SBIN}; do \
		(echo ">>> $${entry}"; \
                 install -c -o root -g bin -m 550 $${entry} \
			/usr/local/sbin/); \
        done;

