#!/usr/bin/perl 
# Скрипт для генерирования index.html в каталогах YYYY.MM
use Getopt::Std;

# Debug level :)
my $debug = 0;

# Корневой каталог, где находятся диры в формате YYYY.MM
my $rdir = "/home/mil/devel/html/out";

# Путь к файлу с шаблоном, на основе которой будет 
# генерироваться index.html
my $pattern = "$rdir/../index.html";

my %opts=(
	A => 0, # Обработка всех каталагов из $rdir, формата YYYY.MM
	d => 0, # Дебуг левел (0 - не использовать отладку)
	r => "", # Указать рутовый каталог ($rdir)
	f => "", # Обработать указанный каталог
	i => "" # Указать путь к шаблону (index.html)
);
getopts('dAf:r:i:', \%opts);

if ( $opts{"d"} == 1 || $debug > 0 ) {
	foreach $key (keys %opts) {
		print "[debug] Getopt: key $key = $opts{$key}\n"
	}
}

if ( $opts{"r"} ne "" ) { $rdir = $opts{"r"} }
die "Root directory $rdir doesn't exist!\n" if !-d $rdir;

if ( $opts{"i"} ne "" ) { $pattern = $opts{"i"} }
die "Pattern: $pattern not a file!\n" if !-f $pattern;

if ( $opts{"d"} == 1 || $debug > 0 ) {
	print << "EOF";

Root directory: $rdir
Pattern file: $pattern
EOF
}

if ( $opts{"f"} ne "" && -d $opts{"f"} ) {
	&pdir($opts{"f"});
} elsif ( $opts{"A"} != 0 ) {
	&adir();
} else {
	&usage();
}

exit;

sub usage {

$usage = "
Usage: $0 [-r path to root dir] [-d] [-i pattern] <-A|-f directory>
-A Process all directories from root dir
-f Process specified directory
-r Specify the root directory
-d Enable debug messages
-i Specify path to pattern file

";
	die $usage;
}

sub adir {
	my $dir = "";
	opendir(DIR, $rdir) or die "Can't opendir $rdir: $!\n";
	while( defined ($dir = readdir(DIR)) ) {
		$dir = "$rdir/$dir";
		next if -f $dir or $dir =~ m|\/\.{1,2}$|g;
		&pdir($dir);
	}		
	closedir(DIR);
}

sub pdir {
	my $dir = shift;
	return if !-d $dir or $dir =~ m|\/\.{1,2}$|g;
	print "==> $dir\n";
	chdir $dir;

	my $nfile = "";

	opendir(DIR, ".") or die "Can't opendir .: $!\n";
	while( defined ($nfile = readdir(DIR)) ) {
		$nfile = "./$nfile";
		next if !-f $nfile or (-f $nfile 
			&& $nfile !~ m/^\.\/\d+\.htm.?$/g);
		push @files, $nfile;

	}		
	closedir(DIR);

	my $total = $#files;

	for ($i = $total; $i != 0; $i--) {
		print "$files[$i]\n";
	}


#	foreach $file (@files) {

#print "($file)\n";

#	}
#print "Files: $#files\n";
	

#		my $arr = "";
#		open(F, "< $nfile") || die "Can't open $nfile: $!\n";
#		while(<F>) { chomp; $arr .= $_; }
#		close(F);

#		my $topic = "";
#		if ( $arr =~ m/<meta name=\"news\" content=\"(.*?)\">/gims ) {
#			$topic = $1;
#		}

#		my $author = "";
#		if ( $arr =~ m|<li><strong>Author:</strong> (.*?)</li>|gims ) {
#			$author = $1;
#		}

#		my $date = "";
#		if ( $arr =~ m|<li><strong>Date:</strong> (.*?)</li>|gims ) {
#			$date = $1;
#		}

#print "-- au: $nfile\n";
#die $arr."\n---\n";

}

sub template {
	my ($filename, $fillings) = @_;
	my $text;
	local $/;
	local *F;
	open(F, "< $filename") || return;
	$text = <F>;
	close(F);
    
	$text =~ s{ %% ( .*? ) %% }
		{ exists ($fillings->{$1}) 
			? $fillings->{$1} 
			: "" 
		}gsex;
    	return $text;
}
