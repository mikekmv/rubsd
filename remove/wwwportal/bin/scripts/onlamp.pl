#!/usr/bin/perl
# url: http://www.onlamp.com/onlamp/general/bsd.csp
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

log("$0");

$url = "http://www.onlamp.com/onlamp/general/bsd.csp";
log ("Location: $url");

if ( !(@C = gethttp($url)) ) {
	log ("Socket error!");
	die "Socket error: $url\n"
}

$data .= $_ foreach (@C);

my $regex = ""; while (<DATA>) { chop; $regex .= $_; } 

$matches = 0;
while ($data =~ m|$regex|gism) {
	++$matches;

	log ("$2");

print << "EOF";

Url: $1 
Title: $2
Author: $3
Bref: $4

EOF
}

if ($matches == 0) {
	log ("Warning! No matches! It's time to check out an regex!");
	die "No matches!\n";
} else { 
	print "STAT: ",length($data), " bytes, $matches matches\n"; 
	log ("Lenght: ",length($data), " bytes, $matches matches");
}

# This is an regular expression for 
# http://www.onlamp.com/onlamp/general/bsd.csp
__DATA__
<p class=\"secondary\"><b>[\x0d\x0d\x0a]+
<a href=\"(.*?)\">(.*?)</a></b>[\x0d\x0d\x0a]+
 by <a href=\".*?\">(.*?)</a>[\x0d\x0d\x0a]+
<br \/>[\x0d\x0d\x0a]+
(.*?)\&nbsp;<i>.*?</i></p>[\x0d\x0d\x0a]+
