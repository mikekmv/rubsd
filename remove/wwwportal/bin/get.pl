#!/usr/bin/perl
# We need to put this script to crontab
# descr: get.pl is used to fork scripts, nothing more :)
# mailto: null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

$scripts = "/home/mil/cvs/wwwportal/bin/scripts"; 

-d $scripts || die "$!: \"$scripts\"\n"; 

$SIG{CHLD}='IGNORE'; # Avoid zombie

&log("Started news tool");

my $i=0;
foreach (<$scripts/*.pl>)
{
	chomp;
	my $script = $_;
	++$i;
	
	&log("Found script: $script");

	unless (fork) {
	  unless (fork) {
		if ( system("/usr/bin/perl $script 2>&1 >/dev/null") != 0 ) { 
			&log("$script failed!");
		}
		exit 0;
	  }
	exit 0;
	}
}

$i == 0 ? 
	&log ("Finished. No scripts found.") : 
	&log ("$i script(s) forked");
exit;
