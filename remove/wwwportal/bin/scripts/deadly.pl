#!/usr/bin/perl
# url: http://www.deadly.com
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

log ("$0");

$url = "http://www.deadly.org/index.php3";
$comments = "Deadly - OpenBSD Journal";
&dbinit($url, $comments);

log ("Location: $url");

if ( !(@C = gethttp($url)) ) {
	my $error = "socket error: $url";
	log ($error);
	die "$log\n";
}

$data .= $_ foreach (@C);

my $regex = ""; while (<DATA>) { chomp; $regex .= $_; } 

$matches = 0;
while ($data =~ m|$regex|gism && ++$matches) {
	my $title = $1;
	my $author = $2;
	my $bref = $3;
	$bref =~ s/[\n\t]+//gims;
	$bref =~ s/\s{2,}/ /gims;
	&dbput ( $title, $bref );
}

$matches == 0 ? log ("Warning! No matches! It's time to check out an regex!") :
	log ("Length: ", length($data), " bytes, $matches matches, fresh = $fresh");

# Regular expression
__DATA__
.*?face="helvetica">\n
<b>(.*?)<\/b>\n
<\/font>\n
<\/td><\/tr>\n
<\/table>\n
<TABLE WIDTH=\"100%\" CELLPADDING=0 CELLSPACING=0 BORDER=0><TR>\n
<TD><FONT SIZE=\"2\" FACE=\"arial,helvetica\"> Contributed by <a href=\".*?\">(.*?)<\/a>.*?<P><FONT FACE=\"arial\,helvetica\">(.*?)<\/FONT>.nbsp\;<\/TD>
