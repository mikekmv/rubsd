#!/usr/bin/perl -w
# We need to put this script to crontab
# descr: get.pl is used to fork scripts, nothing more :)
# mailto: null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

$dir = "/home/mil/cvs/wwwportal/bin/scripts"; 
-d $dir || die "$!: \"$dir\"\n"; 

&log("Started news tool");

my $i=0;
opendir(SCRIPTS, "$dir") || die "Can't open directory $dir\n";
while( my $script = readdir(SCRIPTS) )
{
	if( !(-d $script) && $script =~ m/\.pl$/g ) 
	{
		++$i;
		my $filename = "$dir/$script";
	
		&log("Found script: $filename");
		&chld($filename);
	}
}
closedir(SCRIPTS);

exit;

$i == 0 ? 
	&log ("Finished. No scripts found.") : 
	&log ("$i script(s) forked");
exit;

$SIG{CHLD}='IGNORE'; # Avoid zombie

sub chld {
	my $script = shift; return () if ( !(-f $script) );
	unless (fork) {
 	 if ( system("/usr/bin/perl $script 2>&1 >/dev/null") != 0 ) { 
		&log("$script failed!");
	 }
 	 exit 0;
	}
}