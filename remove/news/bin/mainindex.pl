#!/usr/bin/perl -w

$template_dir="../templates";
$template_file="index.tmpl";
$html_file="index.html";
$html_tempfile="index.tmp";
$html_tempdir="/tmp";
$html_dir="/home/openbsd/cruz/public_html/news";

$template="$template_dir/$template_file";
$temp="$html_tempdir/$html_tempfile";
$file="$html_dir/$html_file";

chomp($day=`date "+%d"`);
chomp($year_month=`date "+%Y.%m"`);
$openbsd_count=0;
$dir="/home/www/openbsd/mail.openbsd.ru/lists/openbsd/$year_month";
while (<$dir/*>) {
  chomp;
  open(R,"<$_") || die "$_:$!\n";
  #local undef $/;
  while(<R>) {
    if (m/^<[^>]+>Date:<[^>]+>.*/) {
     ($tmp,$tmp,$date,$tmp) = split(/ /,$_,4);
     if ($date == $day) 
        { $openbsd_count++; print "1"; }
    }
  }
  close(R);
}
# Correct typos, preserving case
open(TMPL, "< $template")       or die "can't open $template: $!";
open(TMP, "> $file")    or die "can't open $temp: $!";

while (<TMPL>) {
  chomp;
  s/\{TEST\}/\<b\>Нет новостей!\<\/b\>/i;
  s/\{OPENBSD_COUNT\}/$openbsd_count/i;
  s/\{YEAR_MONTH\}/$year_month/i;
  (print TMP $_)		or die "can't write to $temp: $!";
}

close(TMPL)			or die "can't close $template: $!";
close(TMP) 			or die "can't close $temp: $!";

#system("mv",$temp, $file)	or die "can't rename $temp to $file: $!";
