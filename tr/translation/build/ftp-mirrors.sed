#n
/a name=ftp/,/a name=http/ {
	/^<table>/,/^<\/table>/w build/mirrors.ftp
}
/a name=http/,/a name=afs/ {
	/^<table>/,/^<\/table>/w build/mirrors.http
}
/a name=afs/,/a name=rsync/ {
	/^<ul>/,/^<\/ul>/w build/mirrors.afs
}
/a name=rsync/,$ {
	/^<table>/,/^<\/table>/w build/mirrors.rsync
}
