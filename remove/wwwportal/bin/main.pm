package main;
require Exporter;
@ISA = (Exporter);
@EXPORT = qw(gethttp dbinit dbclose dbput log error fresh);

use DBI;

$log = "/home/mil/cvs/wwwportal/bin/logs/news.log";
$dsn = "DBI:mysql:bsdnews:wheel";
$dblogin = "news";
$dbpasswd = ",kz";

$error = "";

my $dbh = "";
my $url_id = 0;
$fresh = 0;

# dbinit( url [, comments] )
# * comments is optional
# descr: connecting to mySQL, gettin' the url id to proccessing
# with `news' table.

sub dbinit {
	my ($url, $comments) = @_;
	&dbopen();

	if ( $url eq "" ) {
		$error = "dbinit(): Specify URL first!";
		&trap();		
	}

	$url = $dbh->quote($url);
	$comments = $dbh->quote($comments);

	my $sth = $dbh->prepare("
		SELECT id FROM source WHERE url = $url
	");
	$sth->execute;

	if ($sth->rows == 0) {
		$dbh->do("
			INSERT INTO source (url, descr, date) VALUES
			($url, $comments, NOW())

		");
		$url_id = $dbh->{'mysql_insertid'};

	} else { $url_id = int ($sth->fetchrow()); }

	$sth->finish;
}

# dbput ( topic [, intro, author] )
# * intro and author is optional
# descr: put the fresh news to the `news' table :)

sub dbput
{
	my ($topic, $intro, $author) = @_;

	if ( $topic eq "" ) {
		$error = "dbput(): Can't find topic! Trap! :)";
		&trap();
	}

	$topic = $dbh->quote($topic);
	$intro = $dbh->quote($intro);
	$author = $dbh->quote($author);

	my $sth = $dbh->prepare("
		SELECT s.id FROM news n, source s 
		WHERE s.id = n.sourceid AND
		s.id = $url_id AND n.topic = $topic
	");
	$sth->execute;

	if ($sth->rows == 0) {
		# This news not found, add it.

		$dbh->do("
			INSERT INTO news (sourceid, author, date, 
			topic, intro) VALUES ($url_id, $author, NOW(),
			$topic, $intro)
		");
		++$fresh;
		&log("[$url_id] $topic");
	}

	$sth->finish;
}

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
	if (!defined ($dbh->disconnect) ) {
		$error = "Can't disconnect from mySQL!";
		&trap();
	}
}

sub log 
{
	if (scalar(@_) == 0) {
		return 0;
	}
	open W, ">>", $log || die "$log: $!\n";
	my $date = `/bin/date '+%d-%m-%Y %H:%M:%S'`; chomp $date;
	print W "$date [$$]: ", @_, "\n";
	close W;
	return 1;
}

sub gethttp 
{
	use IO::Socket;
	my ($url) = $_[0]; return if (!$url);
	my ($host, $args);
	$url =~ s/http:\/\/|\s+//g;
	if ($url =~ /^(.*?)\/(.*?)$/gi) { $host = $1; $args = $2; }	
 	return 0 if (!$host || !$args);

	my $socket = IO::Socket::INET->new(
		PeerAddr => $host,
		PeerPort => 80,
		Proto => "tcp",
		Type => SOCK_STREAM
	) || return 0;

	my $header = "".
	"GET http://$url HTTP/1.0\n".
	"Referer: http://foobar.com\n".
	"Host: $host\n".
	"Accept: */*\n".
	"Accept-Encoding: gzip, deflate\n".
	"Connection: Keep-Alive\n".
	"User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT)\n\n";
	print $socket $header;
	my @sk = <$socket>;
	close($socket);
	return @sk;
}

sub trap 
{
	$error = ( $error eq "" ) ? "Unknown error" : $error;
	&dbclose() if (defined ($dbh) );
	&log ($error); 
	die "$error\n";
}

1;
