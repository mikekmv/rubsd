#!/usr/bin/perl
# url: http://freebsddiary.org
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

log("$0");

$url = "http://www.freebsddiary.org/index.php";
$comments = "The FreeBSD Dairy";
&dbinit($url, $comments);

log ("Location: $url");

if ( !(@C = gethttp($url)) ) {
	log ("Socket error!");
	die "Socket error: $url\n"
}

$data .= $_ foreach (@C);
my $regex = ""; while (<DATA>) { chomp; $regex .= $_; } 

if ($url =~ /^(http:\/\/.*?)\//) {
	$domain = $1;
} 

$matches = 0;
while ($data =~ m|$regex|gism) {
	++$matches;

	my $link = "$domain/$1";
	my $title = $2;

	&dbput ( $title, "New article on The FreeBSD Dairy - \"$title\" available at: $link")
}

if ($matches == 0) {
	log ("Warning! No matches! It's time to check out an regex!");
} else { 
	log ("Lenght: ",length($data), " bytes, $matches matches, fresh: $fresh");
}

&dbclose();
exit;

# Regular expression
__DATA__
\s+<TR>\n
\s+<TD CLASS=\"sans\" NOWRAP VALIGN=\"top\">.*?<\/TD>\n
\s+<TD WIDTH=\"100\%\"><a href=\"(.*?)\">(.*?)<\/a>.*?<\/TD>\n
\s+<\/TR>\n