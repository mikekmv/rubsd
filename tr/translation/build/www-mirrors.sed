#n
#	/^<a href/w build/mirrors.www
#	/^<a href/p
/^Mirrors, by country:/,/^<hr>/ {
	/^<a href/s/">/\/ru\/">/p
}
