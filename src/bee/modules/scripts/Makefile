# $RuOBSD: Makefile,v 1.3 2001/09/12 05:03:21 tm Exp $

SCRIPTS = beetraff-ipf.sh beeapply.sh newlogin.sh 
SBIN    = autoreg.sh

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
	@for entry in ${SCRIPTS}; do \
		(echo ">>> $${entry}"; \
                 install -c -o root -g bin -m 550 $${entry} \
			/usr/local/bin/); \
        done;

