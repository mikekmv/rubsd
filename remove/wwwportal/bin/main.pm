package main;
require Exporter;
@ISA = (Exporter);
@EXPORT = qw(gethttp log);

$log = "/home/mil/cvs/wwwportal/bin/logs/news.log";

sub log {
	if (scalar(@_) == 0) {
		return 0;
	}
	open W, ">>", $log || die "$log: $!\n";
	my $date = `/bin/date '+%d-%m-%Y %H:%M:%S'`; chomp $date;
	print W "$date [$$]: ", @_, "\n";
	close W;
	return 1;
}

sub gethttp {
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

1;
