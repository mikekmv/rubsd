#!/usr/bin/perl
# $Id$
# url: http://slashdot.org/bsd/index.shtml
# null@openbsd.ru

use lib '/home/mil/devel';
use main;

log("$0");

$url = "http://slashdot.org/bsd/index.shtml";
$comments = "Slashdot. News for Nerds, Stuff that metters.";
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
	my $title = $1;
	my $author = $2;
	my $bref = $3;

	&dbput ( $title, $bref, $author )
}

if ($matches == 0) {
	log ("Warning! No matches! It's time to check out an regex!");
} else { 
	log ("Lenght: ",length($data), " bytes, $matches matches, fresh: $fresh");
}

&dbclose();
exit;

# This is an regular expression for 
# http://slashdot.org/bsd/index.shtml
__DATA__
\s+FACE=\"arial,helvetica\" SIZE=\"4\" COLOR=\"#FFFFFF\"><B>(.*?)<\/B><\/FONT><\/TD>\n
<\/TR><\/TABLE><A\n
\s+HREF=\".*?\"><IMG\n
\s+SRC=\".*?\"\n
\s+WIDTH=\"\d+\" HEIGHT=\"\d+\" BORDER=\"0\"\n
\s+ALIGN=\"RIGHT\" HSPACE=\"20\" VSPACE=\"10\"\n
\s+ALT=\".*?\"><\/A><B>Posted by\n
\s+<A HREF=\".*?\">(.*?)<\/A>\n
\s+on .*?<\/B><BR>\n
\s+<FONT SIZE=\"2\"><B>.*?<\/B><\/FONT><BR\n
>(.*?)<P><B>\(<\/B>\n
