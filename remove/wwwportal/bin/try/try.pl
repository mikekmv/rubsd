#!/usr/bin/perl
# Script for testing an regex
# null@openbsd.ru

use lib '/home/mil/cvs/wwwportal/bin';
use main;

# Url for testing
$url = "http://daily.daemonnews.org/index.php3";
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

#my $link = "$domain$1";
my $title = $1;
#my $bref = $3;
#$bref =~ s/[\n\t]+//gims;
#$bref =~ s/\s{2,}/ /gims;

print << "EOF";

Link: $link
Title: $title
Bref: $bref

EOF
}

$matches == 0 ? die "No matches!\n" :
	print "Length: ",length($data), " bytes, $matches matches\n";

# Regular expression
__DATA__
<STRONG><FONT COLOR="#.*?" CLASS="normal">(.*?)</FONT></STRONG>\n
</TD><TD ALIGN=RIGHT BGCOLOR="#.*?">\n
 &nbsp;\n\n
[\s\t]+</TD>\n
[\s\t]+<TD BGCOLOR="#.*?" ALIGN="RIGHT" VALIGN="TOP">\n\n
<IMG ALT="*" .*? HSPACE="0"\n
VSPACE="0" BORDER="0">\n\n
[\s\t]+</TD>\n
[\s\t]+</TR>\n
[\s\t]+<TR>|N
					<TD VALIGN="BOTTOM" ALIGN="LEFT" BGCOLOR="#<!-- nop -->">

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<A HREF="<!-- nop -->"><FONT COLOR="#000000" CLASS="normal"><!-- nop --></FONT></A>

					</TD>
					<TD VALIGN="BOTTOM" ALIGN="RIGHT" BGCOLOR="#<!-- nop -->" NOWRAP>

<FONT COLOR="#000000" CLASS="normal"><!-- nop --></FONT>

					</TD>
					<TD BGCOLOR="#dadada">

&nbsp;&nbsp;

					</TD>
				</TR>
				</TABLE>
					<TABLE WIDTH=100% BORDER=0 CELLPADDING=1>
					<TD BGCOLOR="black">
					<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=3 BGCOLOR="#ffffff" WIDTH=100%><TR><TD>

<FONT COLOR="#000000" CLASS="normal">


Submitted By :

<!-- author -->
<BR>
<!-- bref --></FONT>
<BR><BR>
<STRONG>


<A HREF="<!-- link -->"><FONT COLOR="#000000" CLASS="normal">( Link )</FONT></A>


</STRONG>

&nbsp;&nbsp;&nbsp;


<STRONG>
