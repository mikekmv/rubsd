#!/usr/bin/perl
# Script for testing an regex
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

# Url for testing
$url = "http://www.onlamp.com/onlamp/general/bsd.csp";
$temp = "./try.html";

if (-e $temp) {
	open R, "< $temp" || die "$!\n";
	$data .= $_ while (<R>);
	close R;
} else {
	print "$temp not found, getting $url to $temp\n";
	die "Socket error: $url\n" if ( !(@C = gethttp($url)) );
	open W, ">", $temp || die "$!\n";
	print W $_ foreach (@C);
	close W;
	print "Done\n";
	exit;
}

my $regex = ""; while (<DATA>) { chomp; $regex .= $_; } 

if ($url =~ /^(http:\/\/.*?)\//) {
	$domain = $1;
} 

$matches = 0;
while ($data =~ m|$regex|gism) {
	++$matches;

print << "EOF";

Url: $domain$1 
Title: $2
Author: $3
Bref: $4

EOF
}

$matches == 0 ? die "No matches!\n" :
	print "Length: ",length($data), " bytes, $matches matches\n";

# Regular expression
__DATA__
<p class=\"secondary\"><b>[\x0d\x0d\x0a]+
<a href=\"(.*?)\">(.*?)</a></b>[\x0d\x0d\x0a]+
 by <a href=\".*?\">(.*?)</a>[\x0d\x0d\x0a]+
<br \/>[\x0d\x0d\x0a]+
(.*?)\&nbsp;<i>.*?</i></p>[\x0d\x0d\x0a]+
