#!/usr/bin/perl
# url: http://www.onlamp.com/onlamp/general/bsd.csp
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

log("$0");

$url = "http://www.onlamp.com/onlamp/general/bsd.csp";
$comments = "BSD Articles";
&dbinit($url, $comments);

log ("Location: $url");

if ( !(@C = gethttp($url)) ) {
	log ("Socket error!");
	die "Socket error: $url\n"
}

$data .= $_ foreach (@C);

my $regex = ""; while (<DATA>) { chop; $regex .= $_; } 

if ($url =~ /^(http:\/\/.*?)\//) {
	$pre = $1;
} 

$matches = 0;
while ($data =~ m|$regex|gism) {
	++$matches;
	my $title = $2;
	my $author = $3;
	my $bref = $4;

	my $more = "$pre$1"; # Url to full article
	# rewrite url to `print' version
	$more =~ s|\/pub\/(.*?)\/|\/lpt\/$1\/\/|gims; 

	&dbput ( $title, $bref."\n\nMore: $more", $author )
}

if ($matches == 0) {
	log ("Warning! No matches! It's time to check out an regex!");
} else { 
	log ("Lenght: ",length($data), " bytes, $matches matches, fresh: $fresh");
}

&dbclose();
exit;

# This is an regular expression for 
# http://www.onlamp.com/onlamp/general/bsd.csp
__DATA__
<p class=\"secondary\"><b>[\x0d\x0d\x0a]+
<a href=\"(.*?)\">(.*?)</a></b>[\x0d\x0d\x0a]+
 by <a href=\".*?\">(.*?)</a>[\x0d\x0d\x0a]+
<br \/>[\x0d\x0d\x0a]+
(.*?)\&nbsp;<i>.*?</i></p>[\x0d\x0d\x0a]+
