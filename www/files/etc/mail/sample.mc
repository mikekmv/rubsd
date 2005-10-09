VERSIONID(`$RuOBSD: sample.mc,v 1.20 2005/10/09 10:34:15 form Exp $')dnl
dnl
OSTYPE(openbsd)dnl
dnl
dnl ��������� ������������.
dnl
define(`confPRIVACY_FLAGS', `authwarnings,needmailhelo,noexpn,novrfy')dnl
dnl
dnl UUCP - ��������� ��������. ��������� ������������� UUCP �������.
dnl
FEATURE(nouucp, `reject')dnl
dnl
dnl ��������� ������������ /etc/mail/access ���� ���� ����������.
dnl
FEATURE(`access_db', `hash -o -T<TMPF> /etc/mail/access')dnl
FEATURE(`blacklist_recipients')dnl
dnl
dnl �������� ���������� �� ������ ������� DNS.
dnl
dnl FEATURE(`dnsbl', `list.dsbl.org', `Rejected - see http://dsbl.org/faq-listed/')dnl
dnl FEATURE(`dnsbl', `relays.ordb.org', `Rejected - see http://www.ordb.org/')dnl
dnl FEATURE(`dnsbl', `bl.spamcop.net', `"Spam blocked - see: http://spamcop.net/bl.shtml?"$&{client_addr}')dnl
dnl
dnl ��������� ������������ /etc/mail/local-host-names
dnl
FEATURE(`use_cw_file')dnl
dnl
dnl ��������� ������������ /etc/mail/mailertable � /etc/mail/virtusertable
dnl ���� ����� ����������.
dnl
FEATURE(`mailertable', `hash -o /etc/mail/mailertable')dnl
FEATURE(`virtusertable', `hash -o /etc/mail/virtusertable')dnl
dnl
dnl ��������� ��� ������ � ����� ����������� ���� ���� ���������
dnl ����� �������� ������ �����.
dnl
FEATURE(always_add_domain)dnl
dnl
dnl ��������� ������ redirect ������ ��� ����������� �������� ������.
dnl
FEATURE(redirect)dnl
dnl
dnl ��������� ������� � ��������� ������ ��� ����������� ��������.
snl ��. man smrhs ��� ��������� ����������.
dnl
FEATURE(`smrsh')dnl
dnl
dnl ��������� TLS ��� ������ SMTP ����������. ��� ����� ��������� ����������
dnl �������� starttls(8).
dnl
define(`CERT_DIR', `MAIL_SETTINGS_DIR`'certs')dnl
define(`confCACERT_PATH', `CERT_DIR')dnl
define(`confCACERT', `CERT_DIR/CAcert.pem')dnl
define(`confSERVER_CERT', `CERT_DIR/mycert.pem')dnl
define(`confSERVER_KEY', `CERT_DIR/mykey.pem')dnl
define(`confCLIENT_CERT', `CERT_DIR/mycert.pem')dnl
define(`confCLIENT_KEY', `CERT_DIR/mykey.pem')dnl
dnl
dnl ��������� SMTP ����������� (sendmail ������ ���� ������ � cyrus-sasl2).
dnl
dnl ��������� ������������ ����� ����������� PLAIN �� �������������
dnl ����������.
dnl
define(`confAUTH_OPTIONS', `p')dnl
dnl
dnl ������ ���������� ������� �����������.
dnl
define(`confAUTH_MECHANISMS', `DIGEST-MD5 CRAM-MD5 PLAIN LOGIN')dnl
TRUST_AUTH_MECH(`DIGEST-MD5 CRAM-MD5 PLAIN LOGIN')dnl
dnl
dnl ������������ clamav-milter ��� �������� ����� �� ������� �������
dnl
dnl INPUT_MAIL_FILTER(`clamav', `S=inet:1025@127.0.0.1, F=T, T=S:4m;R:4m')dnl
dnl
dnl ������������ mail.buhal ������ mail.local ��� �������� �����
dnl � Maildir �������������
dnl
dnl define(`LOCAL_MAILER_PATH', `/usr/libexec/mail.buhal')dnl
dnl MODIFY_MAILER_FLAGS(`LOCAL', `-m')dnl
dnl
dnl ������� ���������� SSL ���� 465 (������ SSL ��� STARTTLS)
dnl
dnl DAEMON_OPTIONS(`Name=MTA')dnl
dnl DAEMON_OPTIONS(`Port=465, Name=MTA-SSL, M=s')dnl
dnl
dnl ������ �������� �������
dnl
MAILER(local)dnl
MAILER(smtp)dnl
dnl
dnl
LOCAL_CONFIG
#
# ������ �������, ��������� ������� � "Reveived:" � ������� ������������
# ��������� �� ����. ��. ������� "CheckReceived" ����.
#
# ��������: ����������� �� ������ ���������� ������/���������, �
# ������� ��������� � ���� `Received:'. �������� �����������������
# ������������ (�������� domain.com ��������� ��� �� super.domain.com
# ��� � �� domain.computer.org). ��� ������� ������� ������������ ������
# � ������� ������ ���� ���� ����� ����� ����� ���������� ���� (��������
# ����� MX'� ��� forward).
#
#FS-o /etc/mail/spam-domains

#
# ���������� ��������� ��� ���������� ���������� �����:
#
# - ������, ���������� ����� ���� ������� � �����
# - ������ � ����� �������� ����, ������������ �������
# - ������ � ������� ����� �������
# - ������, ���������� ������ ��� ����� ���� ������
# - ������ � �������� ������ ���� 4 ������
# - ������, ���������� � ����� "dsl", "pppoe", "dial", "dynamic", "dhcp"
#
# ��. ���� ������� "Basic_check_relay".
#
#Kcheckhost regex -a<MATCH> (.+-.+-.+|[0-9]+\.[0-9]+\.[0-9]+\.|[0-9]+-[0-9]+|[0-9]{4}|[^.]+\.[^.]+\.[^.]+\.[^.]+\.|dsl|pppoe|dial|dynamic|dhcp)

LOCAL_RULESETS
#
# �������� ���������
#
HMessage-Id: $>CheckMessageId
#HReceived: $>+CheckReceived

#
# �� ���������� ������ � ������������ �������� Message-Id
#
SCheckMessageId
R< $+ @ $+ >		$@ OK
R$*			$#error $: 553 Header error

#
# ��������� "Received:" �� ���������� �� ������� ������� �� �����
# `/etc/spam-domains'
#
#SCheckReceived
#R$* $=S $*		$#error $@ 5.7.1 $: "550 Access denied"

#
# �� ���������� ������, ����������� �� �������� ��� ����� ��� � ������,
# �� ��������������� IP ������. �� ��������� ������� ������������ ������
# ������ ������������� ��������� ���� (� ������ ������/����, ������� �����
# ���������� ����� ����� ������ ������) ��������� � ����� `/etc/mail/access'
# � ����:
#
# Connect:192.168	OK
# Connect:81.1.212.10	OK
#
#SBasic_check_relay
#R$*			$: < $&{client_resolve} >
#R< TEMP >		$#error $@ 4.7.1 $: "450 Access temporarily denied. Cannot resolve PTR record for " $&{client_addr}
#R< FAIL >		$#error $@ 5.7.1 $: "550 Access denied. IP name lookup failed " $&{client_addr}
#R< FORGED >		$#error $@ 5.7.1 $: "550 Access denied. IP name possibly forged " $&{client_addr}
#
# ��������� ��� ����� �� ����������� ��������� ����
#
#R$*			$: $&{client_name}
#R$*			$: $(checkhost $1 $)
#R< MATCH >		$#error $@ 5.7.1 $: "550 Access denied"
