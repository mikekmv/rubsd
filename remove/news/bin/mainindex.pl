#!/usr/bin/perl -w

my $base_dir="/home/openbsd/cruz/news";
my $lib_dir="$base_dir/lib";

use lib '/home/openbsd/cruz/news/lib';

use mail;
use strict;
use Fcntl qw(:DEFAULT :flock);

my $template_dir="$base_dir/templates";
my $template_file="index.tmpl";
my $html_file="index.html";
my $html_dir="/home/openbsd/cruz/public_html/news";

my $template="$template_dir/$template_file";
my $file="$html_dir/$html_file";
my ($day, $year_month);

chomp($day=`/bin/date "+%d"`);
chomp($year_month=`/bin/date "+%Y.%m"`);

my $openbsd_count=countmail("openbsd",$year_month,$day);

$openbsd_count++ if $openbsd_count < 0; # Actualy we do not nedd to publish this! 

# Correct typos, preserving case
open(TMPL, "< $template")       or die "can't open $template: $!";
my (@temp);

@temp = <TMPL>;

foreach  (@temp) {
  s/\{TEST\}/\<b\>Нет новостей!\<\/b\>/i;
  s/\{OPENBSD_COUNT\}/$openbsd_count/i;
  s/\{YEAR_MONTH\}/$year_month/i;
}

close(TMPL)			|| die "Can't close $template: $!";

# Update index
open(F, "+< $file")		|| die "Can't open $file: $!";
flock(F, LOCK_EX);		# We need exclusive acces for updating this!
seek(F,0,0)			|| die "Can't seek $file: $!";
(print F @temp)			|| die "Can't update $file: $!";
truncate(F,tell(F))		|| die "Can't truncate $file: $!";
close(F)			|| die "Can't close $file: $!";

exit 0;
