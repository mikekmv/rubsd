# $RuOBSD$

SCRIPTS = beetraff-ipf.sh beeapply.sh newlogin.sh

clean:
all:
install: 
	@echo ">>> Installing scripts <<<"
	@for entry in ${SCRIPTS}; do \
		(echo ">>> $${entry}"; \
                 install -c -o root -g bin -m 550 $${entry} \
			/usr/local/bin/); \
        done;
