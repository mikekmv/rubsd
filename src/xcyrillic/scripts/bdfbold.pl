#!/usr/bin/perl -w
#
# Convert bdf fonts to bold style.
# Copyright (C) 1994-95 Cronyx Ltd.
# Author: Serge Vakulenko, <vak@cronyx.ru>
# Changes Copyright (C) 1995-1997 by Andrey A. Chernov, Moscow, Russia.
#
# This software may be used, modified, copied, distributed, and sold,
# in both source and binary form provided that the above copyright
# and these terms are retained. Under no circumstances is the author
# responsible for the proper functioning of this software, nor does
# the author assume any responsibility for damages incurred with its use.
#
# Changes (C) 2000 by Serge Winitzki: speeded up algorithm by 2x.
#

$pattern{"0"} = "0000";
$pattern{"1"} = "0001";
$pattern{"2"} = "0010";
$pattern{"3"} = "0011";
$pattern{"4"} = "0100";
$pattern{"5"} = "0101";
$pattern{"6"} = "0110";
$pattern{"7"} = "0111";
$pattern{"8"} = "1000";
$pattern{"9"} = "1001";
$pattern{"a"} = "1010";         $pattern{"A"} = "1010";
$pattern{"b"} = "1011";         $pattern{"B"} = "1011";
$pattern{"c"} = "1100";         $pattern{"C"} = "1100";
$pattern{"d"} = "1101";         $pattern{"D"} = "1101";
$pattern{"e"} = "1110";         $pattern{"E"} = "1110";
$pattern{"f"} = "1111";         $pattern{"F"} = "1111";
$pattern{"\n"} = "";            $pattern{"\r"} = "";

$hexdig{"0000"} = "0";
$hexdig{"0001"} = "1";
$hexdig{"0010"} = "2";
$hexdig{"0011"} = "3";
$hexdig{"0100"} = "4";
$hexdig{"0101"} = "5";
$hexdig{"0110"} = "6";
$hexdig{"0111"} = "7";
$hexdig{"1000"} = "8";
$hexdig{"1001"} = "9";
$hexdig{"1010"} = "A";
$hexdig{"1011"} = "B";
$hexdig{"1100"} = "C";
$hexdig{"1101"} = "D";
$hexdig{"1110"} = "E";
$hexdig{"1111"} = "F";

$swidth = $dwidth = -1;

while (<STDIN>) {
	if (/^WEIGHT_NAME\s/) {
		s/"Medium"/"Bold"/;
		print;
	} elsif (/^FONT\s/) {
		s/-Medium-/-Bold-/;
		print;
	} elsif (/^FONTBOUNDINGBOX\s/) {
		@r = split;
		$h = $r[2];
		$w = $r[1] + 1;
		printf "FONTBOUNDINGBOX %d %d %d %d\n", $w, $h, $r[3], $r[4];
	} elsif (/^CHARS\s/) {
		print;
		last;
	} else {
		print;
	}
}

while (<STDIN>) {
	if (/^STARTCHAR\s/) {
		print;
	} elsif (/^ENDCHAR/) {
		print;
	} elsif (/^ENCODING\s/) {
		print;
	} elsif (/^SWIDTH\s/) {
		@r = split;
		$swidth = $r[1];
	} elsif (/^DWIDTH\s/) {
		@r = split;
		$dwidth = $r[1];
	} elsif (/^BBX\s/) {
		@r = split;
		$h = $r[2];
		$w = $r[1];
		$h = int (($dwidth * 1000 / $swidth) + 0.5)
			if ($h == 0 && $dwidth >= 0 && $swidth >= 0);
		$dwidth = $w
			if ($dwidth < 0);
		$dwidth++;
		$w++;
		printf "SWIDTH %d 0\n", int (($dwidth * 1000 / $h) + 0.5);
		printf "DWIDTH %d 0\n", $dwidth;
		$w = 0
			if ($r[1] == 0 && $r[2] == 0);
		$h = 0
			if ($r[2] == 0);
		printf "BBX %d %d %d %d\n", $w, $h, $r[3], $r[4];
	} elsif (/^BITMAP/) {
		$swidth = $dwidth = -1;
		print "BITMAP\n";
		&makechar;
	}
}
print "ENDFONT\n";

sub makechar {
	for ($i=0; $i<$h; ++$i) {
		$b = "";
		foreach $c (split(//,<STDIN>)) {
			$b .= $pattern{$c};
		}
		$b .= "0" x 8;
		for ($n=$w-1; $n>=1; --$n) {
			if (substr($b, $n-1, 2) eq "10") {
				substr($b, $n, 1) = "1";
			}
		}
	
	# Now we are ready with the draft of current line.
		for ($n = 0; $n<2*int(($w+7)/8); ++$n) {
			print $hexdig{substr($b, $n*4, 4)};
		}
		print "\n";
	}
}
