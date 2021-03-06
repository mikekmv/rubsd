#!/usr/bin/perl -w
use DBI;

my $OUT = "/home/mil/devel/html/out";
my $dsn = "DBI:mysql:bsdnews";
my $dblogin = "news";
my $dbpasswd = ",kz";
my $error = "";

&dbopen();

my $sth = $dbh->prepare("
	SELECT n.id, n.topic, n.author, n.intro, n.date, s.url, s.descr
	FROM news n, source s
	WHERE n.sourceid = s.id AND
	n.static = 'n'
	ORDER BY n.date ASC
");
$sth->execute;


if ($sth->rows == 0) {
	$error = "All news is already static!";
	&trap();
}

while (my ($id, $topic, $author, $intro, $date, $url, $descr) 
		= $sth->fetchrow() ) 
{
	$date =~ m|^(\d+)\-(\d+)|g; 
	my $odir = "$OUT/$1.$2";
	
	mkdir $odir if ( !-d $odir );
	
	my $id = &lastid($odir); ++$id;
	open W, "> $odir/$id.html" || die "$!\n";

	$intro =~ s/\n/<br>/g;
	$gdate = `/bin/date`; chomp ($gdate);
	
	if ( !$author || $author eq "" ) {
		$author = "Unknown";
	}

	$descr = $url if ( !$descr || $descr eq "" );
	$intro =~ s/<\/?font.*?>//gims; # cut font tag
	$intro =~ s/<pre>\s*(.*?)/<pre>\n$1/gims; # <pre> align
	$intro =~s/More: (http:.*?)$/More: <a href=\"$1\" target=\"_blank">$1<\/a>/gims;

	chomp($topic);

# >>>>>>> PATTERN <<<<<<<

print W << "EOF";
<!doctype html public "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
    <META NAME="Author" content="$author">
    <META NAME="News" content="$topic">
    <title>tty.net.ru: $topic</title>
</head>
<body bgcolor="#ffffff" link="#35567a" alink="#35567a" vlink="#35567a">

<h2>$topic</h2>

<ul>
    <li><strong>Author:</strong> $author</li>
    <li><strong>Date:</strong> $date</li>
    <li><strong>Source:</strong><a href="$url">$descr</a></li>
</ul>

<p>
$intro
<p>
<hr noshade>
<small>
<em>
This news was generated by <a href="http://tty.net.ru"> gen.pl</a> 
: $gdate
</em>
</small>

</body>
</html>
EOF

# >>>>>>> PATTERN <<<<<<<
	close W;

	$dbh->do("UPDATE news SET static='Y' WHERE id = $id");

}

$sth->finish;
&dbclose();
exit;

sub dbopen 
{
	$dbh = DBI->connect( $dsn, $dblogin, $dbpasswd,
		{ PrintError => 0, RaiseError => 0 }
	);

	if (!defined ($dbh) ) {
		$error = "Can't connect to mySQL: dsn = $dsn";
		&trap();
	}
}

sub dbclose 
{
	$sth->finish;
	if (!defined ($dbh->disconnect) ) {
		$error = "Can't disconnect from mySQL!";
		&trap();
	}
}

sub trap 
{
	$error = ( $error eq "" ) ? "Unknown error" : $error;
	&dbclose() if (defined ($dbh) );
	die "$error\n";
}

sub lastid 
{
	my $dir = shift;
	my $max = 0;

	opendir(NDIR, "$dir") || die "Can't open directory $dir\n";
	while( my $file = readdir(NDIR) )
	{
		if( !-d $file && $file =~ m/^(\d+)\.html$/g ) 
		{
			if ($1 > $max) { $max = $1; }
		}
	}
	closedir(NDIR);
	return $max;
}