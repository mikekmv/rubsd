package mail;
require Exporter;
@ISA = (Exporter);
@EXPORT = qw(countmail);

use strict;

my $mail_dir = "/home/www/openbsd/mail.openbsd.ru/lists";

sub countmail {
  my ($list,$date,$day) = @_;
  my $full_path = "$mail_dir/$list/$date";
  my $mail_count=0;

  return -1 if ! -d $full_path;

  while(<$full_path/*>) {
    chomp;
    next if ! -f $_; # skip, it's not a file
    open(R, "< $_") || die "$_: $!\n";
    while(<R>) {
      if (m/^<[^>]+>Date:<[^>]+>\s*\S+\s+(\d+)\s+/g) {
         $mail_count++ if $1 == $day;
      }
    }
    close(R);
  }
  return $mail_count;
}

sub getlatest {
  my ($list) = @_;
  my $full_path = "$mail_dir/$list";
  return "" if ! -d $full_path;

  return 0;
}

1