# $RuOBSD$
#
# ������ ������ ��� �������������� ���������� ����� �� Dovecot IMAP �������.
# ���� �������� � �������� ������� ��� ������ .dovecot.sieve � �������������
# ��� ���� ����� (��� ������ ��������� dovecot-sieve).
#
# ����� ��������� ���������� ����� ���������� �����:
#	http://wiki.dovecot.org/LDA/Sieve

# ���������� ������ ���������.
#
require ["fileinto", "imapflags", "regex"];

# ������ � �������� openbsd@openbsd.ru ������ � ����� "OpenBSD/ru".
#
if address ["To", "Cc"] ["openbsd@openbsd.ru", "openbsd@cvs.openbsd.ru"] {
	fileinto "OpenBSD.ru";
	stop;
}

# ������ � �������� tech@openbsd.org ������ � ����� "OpenBSD/tech".
#
if address ["To", "Cc"] ["tech@openbsd.org", "tech@cvs.openbsd.org"] {
	fileinto "OpenBSD.tech";
	stop;
}

# ������ �� cron ������� /etc/security �������� ������� ������
# � thunderbird.
#
if header :regex "Subject" ".* daily insecurity output" {
	addflag "$label1";
}
