#!/usr/bin/perl
# url: http://bsdtoday.com 
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

log("$0");

$url = "http://bsdtoday.com/index.html";
$comments = "Your Daily Source for BSD News and Information";
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

	my $more = "$domain$1";
	my $title = $2;
	my $bref = $3;
	$bref =~ s/[\n\t]+//gims;
	$bref =~ s/\s{2,}/ /gims;

	&dbput ( $title, $bref."\n\nMore: $more")
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
<b><a href="(.*?)">(.*?)</a></b>\n
<br>\n
<font size=-1>(.*?)</font><br>\n
<font size=1 color=#505050>.*?</font><br>\n