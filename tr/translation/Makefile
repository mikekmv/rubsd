all: openbsd-www openssh-www sync

openbsd-www:
	@echo "Synchronizing OpenBSD WWW mirrors..."

	@ftp -o - -V http://www.openbsd.org/index.html |\
		sed -f build/www-mirrors.sed |\
		sh build/insert.sh ru/index.html WWW

	@rm -f build/mirrors.www

openssh-www:
	@echo "Synchronizing OpenSSH WWW mirrors..."

	@ftp -o - -V http://www.openssh.com/index.html |\
		sed -f build/www-mirrors.sed |\
		sh build/insert.sh openssh/ru/index.html WWW

	@rm -f build/mirrors.www

sync:
.ifdef STEELIX_CVS
	@echo "Synchronizing RuOBSD CVS with Steelix CVS..."
	@for FILE in `cat commit-list`; do \
		cp -f $$FILE $(STEELIX_CVS)/$$FILE; \
	done
.endif
