all-mirrors: openbsd-ftp openbsd-www openssh-ftp openssh-www

openbsd-anoncvs:
	@echo "Synchronizing OpenBSD AnonCVS mirrors..."

	@sed -f build/anoncvs-mirrors.sed $(OPENBSD_CVS)/anoncvs.html

	@sed -f build/anoncvs.sed -f build/contries.sed build/mirrors.anoncvs |\
		sh build/insert.sh ru/anoncvs.html ANONCVS

	@rm -f build/mirrors.anoncvs

openbsd-ftp:
	@echo "Synchronizing OpenBSD FTP mirrors..."

	@sed -f build/ftp-mirrors.sed $(OPENBSD_CVS)/ftp.html

	@sed -f build/ftp.sed -f build/contries.sed build/mirrors.ftp |\
		sh build/insert.sh ru/ftp.html FTP
	@sed -f build/ftp.sed -f build/contries.sed build/mirrors.http |\
		sh build/insert.sh ru/ftp.html HTTP
	@sed -f build/ftp.sed -f build/contries.sed build/mirrors.afs |\
		sh build/insert.sh ru/ftp.html AFS
	@sed -f build/ftp.sed -f build/contries.sed build/mirrors.rsync |\
		sh build/insert.sh ru/ftp.html RSYNC

	@rm -f build/mirrors.ftp build/mirrors.http \
		build/mirrors.afs build/mirrors.rsync

openbsd-www:
	@echo "Synchronizing OpenBSD WWW mirrors..."

	@sed -f build/www-mirrors.sed $(OPENBSD_CVS)/index.html |\
		sh build/insert.sh ru/index.html WWW

	@rm -f build/mirrors.www

openssh-ftp:
	@echo "Synchronizing OpenSSH FTP mirrors..."

	@sed -n -e '/^<table>/,/^<\/table>/p' $(OPENBSD_CVS)/openssh/ftp.html |\
		sed -f build/ftp.sed -f build/contries.sed |\
		sh build/insert.sh openssh/ru/ftp.html FTP

openssh-www:
	@echo "Synchronizing OpenSSH WWW mirrors..."

	@sed -f build/www-mirrors.sed $(OPENBSD_CVS)/openssh/index.html |\
		sh build/insert.sh openssh/ru/index.html WWW

	@rm -f build/mirrors.www
