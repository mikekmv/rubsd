#!/usr/bin/perl -w
use strict;

my $template_dir="../templates";
my $template_file="index.tmpl";
my $html_file="index.html";
my $html_tempfile="index.tmp";
my $html_tempdir="/tmp";
my $html_dir="/home/openbsd/null/public_html/news";

my $template="$template_dir/$template_file";
my $temp="$html_tempdir/$html_tempfile";
my $file="$html_dir/$html_file";
my ($day, $year_month);

chomp($day=`/bin/date "+%d"`);
chomp($year_month=`/bin/date "+%Y.%m"`);

my $openbsd_count=0;
my $dir = "/home/www/openbsd/mail.openbsd.ru/lists/openbsd/$year_month";

while(<$dir/*>) {
  chomp;
  next if ! -f $_; # skip, it's not a file
  open(R, "< $_") || die "$_: $!\n";
  while(<R>) {
    if (m/^<[^>]+>Date:<[^>]+>\s*\S+\s+(\d+)\s+/g) {
       $openbsd_count++ if $1 == $day;
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
  print TMP $_		or die "can't write to $temp: $!";
}

close(TMPL)			or die "can't close $template: $!";
close(TMP) 			or die "can't close $temp: $!";

exit 0;
