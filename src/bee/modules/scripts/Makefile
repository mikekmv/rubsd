# $RuOBSD: Makefile,v 1.8 2002/06/01 19:11:09 shadow Exp $

SCRIPTS = beetraff-ipf.sh beeapply.sh newlogin.sh beetraff-ipstatd.sh beepfrules.sh
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

